#include "recorder.h"

Recorder::Recorder():
    codecContext(nullptr),
    formatContext(nullptr),
    outStream(nullptr),
    yuv420pFrame(nullptr),
    swsContext(nullptr),
    frame(nullptr),
    packet(nullptr),
    pixelFormat(AV_PIX_FMT_RGB24),
    pts(0),
    state(STATE_NONE)
{

}

Recorder::~Recorder()
{
    clear();
}

int Recorder::start(int width, int height, AVPixelFormat pixelFormat_, const std::string &videoName)
{
    std::lock_guard<std::mutex> guard(mutex);
    if (state == STATE_READY) {
        return 0;
    }
    /* clear */
    clear();
    pixelFormat = pixelFormat_;
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
    codecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    codecContext->bit_rate = 400000; //sharpness
    codecContext->gop_size = 64;
    codecContext->framerate.num = 5;// play speed
    codecContext->framerate.den = 1;
    codecContext->time_base = av_inv_q(codecContext->framerate);
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    codecContext->width = width;
    codecContext->height = height;
    codecContext->sample_aspect_ratio.num = width;
    codecContext->sample_aspect_ratio.den = height;
    if (codecContext->codec_id == AV_CODEC_ID_H264) {
        codecContext->qmin = 10;
        codecContext->qmax = 51;
        codecContext->qcompress = 0.6;
        av_opt_set(codecContext->priv_data, "preset", "slow", 0);
    } else if (codecContext->codec_id == AV_CODEC_ID_MPEG2VIDEO){
        codecContext->max_b_frames = 2;
    } else if (codecContext->codec_id == AV_CODEC_ID_MPEG1VIDEO){
        codecContext->mb_decision = 2;
    } else if (codecContext->codec_id == AV_CODEC_ID_MPEG4){
        codecContext->mb_decision = 1;
    } else {
        std::cout<<"codec id:"<<codecContext->codec_id<<std::endl;
    }

    if (avcodec_open2(codecContext, encoder, nullptr) < 0) {
        std::cout<<"Open encoder failed!"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    /* ooutput */
    int ret = avformat_alloc_output_context2(&formatContext, nullptr, nullptr, videoName.c_str());
    if (ret != 0) {
        std::cout<<"can not alloc outputContext "<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    /* stream */
    outStream = avformat_new_stream(formatContext, nullptr);
    avcodec_parameters_from_context(outStream->codecpar, codecContext);
    ret = avio_open(&formatContext->pb, videoName.c_str(), AVIO_FLAG_WRITE);
    if (ret != 0) {
        std::cout<<"avio_open  failed!\n"<<std::endl;
        return -1;
    }
    /* header */
    if (avformat_write_header(formatContext, nullptr) < 0) {
        std::cout<<"Write header failed!\n"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    /* transcode */
    swsContext = sws_getContext(codecContext->width,
                                codecContext->height,
                                pixelFormat,
                                codecContext->width,
                                codecContext->height,
                                AV_PIX_FMT_YUV420P,
                                SWS_POINT, nullptr, nullptr, nullptr);
    if (swsContext == nullptr) {
        std::cout<<"Could not initialize the conversion context"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    /* rgb frame */
    frame = av_frame_alloc();
    frame->format = pixelFormat;
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
        std::cout<<"Could not allocate AVPacket"<<std::endl;
        state = STATE_NONE;
        return -1;
    }
    state = STATE_READY;
    return 0;
}

int Recorder::rawEncode(unsigned char *data)
{
    {
        std::lock_guard<std::mutex> guard(mutex);
        if (state != STATE_READY) {
            return 0;
        }
        int ret = av_frame_make_writable(yuv420pFrame);
        if (ret < 0) {
            return -1;
        }
        av_image_fill_arrays(frame->data,
                             frame->linesize,
                             data,
                             pixelFormat,
                             codecContext->width,
                             codecContext->height,
                             1);
    }
    return encode(frame);
}

int Recorder::encode(AVFrame *frame)
{
    std::lock_guard<std::mutex> guard(mutex);
    if (state != STATE_READY) {
        return 0;
    }
    sws_scale(swsContext,
              frame->data,
              frame->linesize,
              0,
              codecContext->height,
              yuv420pFrame->data,
              yuv420pFrame->linesize);
    yuv420pFrame->pts = pts++;
    int ret = avcodec_send_frame(codecContext, yuv420pFrame);
    if (ret < 0) {
        std::cout<<"Error sending a frame to the encoder:"<<av_err2str(ret)<<std::endl;
        return -1;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(codecContext, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            std::cout<<"Error encoding a frame: "<<av_err2str(ret)<<std::endl;
            return -1;
        }

        /* rescale output packet timestamp values from codec to stream timebase */
        av_packet_rescale_ts(packet, codecContext->time_base, outStream->time_base);
        packet->stream_index = outStream->index;

        /* Write the compressed frame to the media file. */
        ret = av_interleaved_write_frame(formatContext, packet);
        /* pkt is now blank (av_interleaved_write_frame() takes ownership of
         * its contents and resets pkt), so that no unreferencing is necessary.
         * This would be different if one used av_write_frame(). */
        if (ret < 0) {
            std::cout<<"Error while writing output packet: "<< av_err2str(ret)<<std::endl;
            return -1;
        }
        av_packet_unref(packet);
    }
    return 0;
}

void Recorder::stop()
{
    std::lock_guard<std::mutex> guard(mutex);
    if (state == STATE_NONE) {
        return;
    }
    state = STATE_FINISHED;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    /* flush */
    av_write_trailer(formatContext);
    /* clear */
    clear();
    return;
}

void Recorder::clear()
{
    if (formatContext != nullptr) {
        avio_close(formatContext->pb);
        avformat_free_context(formatContext);
        formatContext = nullptr;
    }
    if (codecContext != nullptr) {
        avcodec_close(codecContext);
        avcodec_free_context(&codecContext);
        codecContext = nullptr;
    }
    if (yuv420pFrame != nullptr) {
        av_frame_free(&yuv420pFrame);
        yuv420pFrame = nullptr;
    }
    if (frame != nullptr) {
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (packet != nullptr) {
        av_packet_free(&packet);
        packet = nullptr;
    }
    if (swsContext != nullptr) {
        sws_freeContext(swsContext);
        swsContext = nullptr;
    }
    state = STATE_NONE;
    pts = 0;
    return;
}
