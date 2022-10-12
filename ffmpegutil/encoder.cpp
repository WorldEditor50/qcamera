#include "encoder.h"

Encoder::~Encoder()
{
    if (context != nullptr) {
        avcodec_close(context);
        avcodec_free_context(&context);
        context = nullptr;
    }
    if (outputContext != nullptr) {
        avformat_close_input(&outputContext);
        avformat_free_context(outputContext);
        outputContext = nullptr;
    }

    if (packet != nullptr) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }
}

bool Encoder::open(int width, int height, const std::string &formatName, const std::string &fileName)
{
    if (formatName.empty() || fileName.empty()) {
        std::cout<<"invalid parameters."<<std::endl;
        return false;
    }
    /* output context */
    int ret = avformat_alloc_output_context2(&outputContext, nullptr, formatName.c_str(), fileName.c_str());
    if (ret != 0) {
        std::cout<<"can not alloc outputContext "<<std::endl;
        return false;
    }
    ret = avio_open2(&outputContext->pb, fileName.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
    if (ret != 0) {
        std::cout<<"can not open outputContext, avio_open2 failed. "<<std::endl;
        return false;
    }
    /* encoder */
    const AVCodec* encodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (encodec == nullptr) {
        std::cout<<"avcodec_find_encoder failed."<<std::endl;
        return false;
    }
    context = avcodec_alloc_context3(encodec);
    if (context == nullptr) {
        std::cout<<"can not allocate encodeContext"<<std::endl;
        return false;
    }
    context->codec_type = AVMEDIA_TYPE_VIDEO;
    context->bit_rate = 400000; //sharpness
    context->gop_size = 64;
    context->framerate.num = 5;// play speed
    context->framerate.den = 1;
    context->time_base = av_inv_q(context->framerate);
    context->pix_fmt = AV_PIX_FMT_YUV420P;
    context->width = width;
    context->height = height;
    context->sample_aspect_ratio.num = width;
    context->sample_aspect_ratio.den = height;
    if (context->codec_id == AV_CODEC_ID_H264) {
        context->qmin = 10;
        context->qmax = 51;
        context->qcompress = 0.6;
        //av_opt_set(context->priv_data, "preset", "slow", 0);
    } else if (context->codec_id == AV_CODEC_ID_MPEG2VIDEO){
        context->max_b_frames = 2;
    } else if (context->codec_id == AV_CODEC_ID_MPEG1VIDEO){
        context->mb_decision = 2;
    } else {
        std::cout<<"codec id:"<<context->codec_id<<std::endl;
    }

    ret = avcodec_open2(context, encodec, nullptr);
    if (ret < 0) {
        std::cout<<"can not open encodeContext "<<std::endl;
        char buf[1024]={0};
        av_strerror(ret, buf, 1024);
        std::cout<<std::string(buf);
        return false;
    }
    /* stream */
    AVStream *outStream = avformat_new_stream(outputContext, nullptr);
    outStream->time_base = context->time_base;
    ret = avcodec_parameters_from_context(outStream->codecpar, context);
    if (ret < 0) {
        std::cout<<"Failed to copy encoder parameters to output stream"<<std::endl;
        return false;
    }
    av_dump_format(outputContext, 0, fileName.c_str(), 1);
    ret = avformat_write_header(outputContext, nullptr);
    if (ret != 0) {
        std::cout<<"encoder: can not write header. code="<<ret<<std::endl;
        return false;
    }

    /* packet */
    packet = av_packet_alloc();
    if (packet == nullptr) {
        std::cout<<"failed to allocate packet."<<std::endl;
        return false;
    }
    return true;
}

void Encoder::operator()(AVFrame *frame)
{
    /* send the frame to the encoder */
    if (frame != nullptr) {
        frame->pts = framePts;
        framePts++;
    }
    int ret = avcodec_send_frame(context, frame);
    if (ret < 0) {
        std::cout<<"avcodec_send_frame failed."<<std::endl;
        char buf[1024]={0};
        av_strerror(ret, buf, 1024);
        std::cout<<std::string(buf);
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(context, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
         } else if (ret < 0) {
            //fprintf(stderr, "Error during encoding\n");
            break;
        }
        AVStream *outStream = outputContext->streams[packet->stream_index];
        av_packet_rescale_ts(packet,
                             context->time_base,
                             outStream->time_base);
        packet->pos = -1;
        packet->stream_index = outStream->index;
        av_interleaved_write_frame(outputContext, packet);
        av_packet_unref(packet);
    }
    return;
}

void Encoder::flush()
{
    this->operator()(nullptr);
    av_write_trailer(outputContext);
    return;
}
