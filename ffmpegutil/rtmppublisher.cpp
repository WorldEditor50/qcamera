#include "rtmppublisher.h"

RtmpPublisher::~RtmpPublisher()
{
    stop();
    if (isInited == true) {
        avformat_network_deinit();
    }
}

void RtmpPublisher::enableNetwork()
{
    /* init network */
    if (isInited == false) {
        int ret = avformat_network_init();
        if (ret != 0) {
            std::cout<<"ffmpeg network invalid"<<std::endl;
        }
        isInited = true;
    }
}

int RtmpPublisher::start(int width, int height, const char* url)
{
    std::lock_guard<std::mutex> guard(mutex);
    if (state == STATE_STREAMING) {
        return 0;
    }
    if (url == nullptr) {
        std::cout<<"invalid parameters."<<std::endl;
        state = STATE_NONE;
        return -1;
    }

    /* output context */
    int ret = avformat_alloc_output_context2(&outputContext, nullptr, "flv", url);
    if (ret != 0) {
        std::cout<<"can not alloc outputContext "<<std::endl;
        state = STATE_NONE;
        return -1;
    }

    /* open url */
    ret = avio_open(&outputContext->pb, url, AVIO_FLAG_READ_WRITE);
    if (ret != 0) {
        std::cout<<"can not open outputContext, avio_open failed. "<<std::endl;
        state = STATE_NONE;
        return -1;
    }

    /* encoder */
    const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (encoder == nullptr) {
        std::cout<<"Find encoder AV_CODEC_ID_H264 failed!"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    codecContext = avcodec_alloc_context3(encoder);
    if (codecContext == nullptr) {
        std::cout<<"Alloc context for encoder contx failed"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    fps = 30;
    codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    codecContext->bit_rate = 400000; //sharpness
    codecContext->gop_size = 50;
    codecContext->framerate.num = fps;// play speed
    codecContext->framerate.den = 1;
    codecContext->time_base = av_inv_q(codecContext->framerate);
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->sample_aspect_ratio.num = width;
    codecContext->sample_aspect_ratio.den = height;

    codecContext->qmin = 10;
    codecContext->qmax = 51;
    codecContext->qcompress = 0.6;

    AVDictionary *param = nullptr;
    av_dict_set(&param, "preset", "superfast", 0);
    av_dict_set(&param, "tune", "zerolatency", 0);
    if (avcodec_open2(codecContext, encoder, &param) < 0) {
        std::cout<<"Open encoder failed!"<<std::endl;
        state = STATE_NONE;
        return -1;
    }

    /* out stream */
    outStream = avformat_new_stream(outputContext, nullptr);
    outStream->time_base.num = 1;
    outStream->time_base.den = fps;
    outStream->codecpar->codec_tag = 0;
    ret = avcodec_parameters_from_context(outStream->codecpar, codecContext);
    if (ret < 0) {
        std::cout<<"Failed to copy Publisher parameters to output stream"<<std::endl;
        state = STATE_NONE;
        return -1;
    }

    /* write header */
    AVDictionary *options = nullptr;
    av_dict_set(&options, "flvflags", "no_duration_filesize", 0);
    //av_dict_set(&options, "rtmp_transport", "tcp", 0);
    //av_dict_set(&options, "stimeout", "8000000", 0);
    ret = avformat_write_header(outputContext, &options);
    if (ret != 0) {
        std::cout<<"Publisher: can not write header. code="<<ret<<std::endl;
        state = STATE_NONE;
        char buf[1024]={0};
        av_strerror(ret, buf, 1024);
        std::cout<<std::string(buf);
        return -1;
    }
    av_dict_free(&options);
    av_dump_format(outputContext, 0, url, 1);
    /* transcode */
    swsContext = sws_getContext(width,
                                height,
                                AV_PIX_FMT_RGB24,
                                width,
                                height,
                                AV_PIX_FMT_YUV420P,
                                SWS_POINT, nullptr, nullptr, nullptr);
    if (swsContext == nullptr) {
        std::cout<<"Could not initialize the conversion context"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    /* rgb frame */
    frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_RGB24;
    frame->width  = width;
    frame->height = height;
    /* yuv frame */
    yuv420pFrame = av_frame_alloc();
    yuv420pFrame->format = AV_PIX_FMT_YUV420P;
    yuv420pFrame->width = width;
    yuv420pFrame->height = height;
    if (av_frame_get_buffer(yuv420pFrame, 0) < 0) {
        av_frame_free(&yuv420pFrame);
        yuv420pFrame = NULL;
        std::cout<<"Frame get buffer failed"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    /* packet */
    packet = av_packet_alloc();
    if (packet == nullptr) {
        std::cout<<"failed to allocate packet."<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    state = STATE_STREAMING;
    /* start time */
    startTime = av_gettime();
    timeBase = codecContext->time_base;
    timeBaseFloat = timeBase.num * 1.0 / timeBase.den;
    return 0;
}

void RtmpPublisher::encode(unsigned char *data)
{
    std::lock_guard<std::mutex> guard(mutex);
    if (state != STATE_STREAMING) {
        return;
    }
    /* rgb => ffmpeg frame */
    av_image_fill_arrays(frame->data,
                         frame->linesize,
                         data,
                         AV_PIX_FMT_RGB24,
                         codecContext->width,
                         codecContext->height,
                         1);
    /* RGB24 => YUV420P */
    sws_scale(swsContext,
              frame->data,
              frame->linesize,
              0,
              codecContext->height,
              yuv420pFrame->data,
              yuv420pFrame->linesize);
    yuv420pFrame->pts = frameCount;
    frameCount++;
    /* send the frame to the encoder */
    int ret = avcodec_send_frame(codecContext, yuv420pFrame);
    if (ret < 0) {
        std::cout<<"avcodec_send_frame failed."<<std::endl;
        char buf[1024]={0};
        av_strerror(ret, buf, 1024);
        std::cout<<std::string(buf);
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
         } else if (ret < 0) {
            //fprintf(stderr, "Error during encoding\n");
            break;
        }
        packet->stream_index = outStream->index;
        //AVRational time_base = outputContext->streams[0]->time_base;
        packet->pts = frameCount*(outStream->time_base.den)/((outStream->time_base.num)*fps);
        packet->dts = packet->pts;
        packet->duration = (outStream->time_base.den) /((outStream->time_base.num)*fps);
        packet->pos = -1;

#if 0
        if(AV_NOPTS_VALUE == packet->pts) {
            av_usleep(32000);
        } else {
            int64_t nowTime = av_gettime() - startTime;
            int64_t pts = packet->pts * 1000 * 1000 * timeBaseFloat;
            if(pts > nowTime) {
                av_usleep(pts - nowTime);
            }
        }

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(packet, codecContext->time_base, outStream->time_base);
        packet->stream_index = outStream->index;
#endif
        /* write frame */
        av_interleaved_write_frame(outputContext, packet);
        av_packet_unref(packet);
    }
    return;
}

void RtmpPublisher::stop()
{
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (state == STATE_NONE) {
            return;
        }
        state = STATE_FINISHED;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (outputContext != nullptr) {
        av_write_trailer(outputContext);
    }
    clear();
    return;
}

void RtmpPublisher::clear()
{
    if (codecContext != nullptr) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }
    if (outputContext != nullptr) {
        avformat_close_input(&outputContext);
        avformat_free_context(outputContext);
        outputContext = nullptr;
    }

    if (yuv420pFrame != nullptr) {
        av_frame_free(&yuv420pFrame);
        yuv420pFrame = nullptr;
    }
    if (frame != nullptr) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (swsContext != nullptr) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }
    if (packet != nullptr) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }
    frameCount = 0;
    return;
}
