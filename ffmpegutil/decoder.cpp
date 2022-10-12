#include "decoder.h"

Decoder::~Decoder()
{
    if (context != nullptr) {
        avcodec_close(context);
        avcodec_free_context(&context);
        context = nullptr;
    }
    if (frame != nullptr) {
        av_frame_unref(frame);
        av_frame_free(&frame);
        frame = nullptr;
    }
    if (packet != nullptr) {
        av_packet_unref(packet);
        av_packet_free(&packet);
        packet = nullptr;
    }
}
bool Decoder::open(AVFormatContext *inputContext_)
{
    if (inputContext_ == nullptr) {
        std::cout<<"invalid inputContext"<<std::endl;
        return false;
    }
    inputContext = inputContext_;
#ifdef WIN64
    AVCodec *codec = nullptr;
#else
    const AVCodec *codec = nullptr;
#endif
    videoStreamIndex = av_find_best_stream(inputContext, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
    if (codec == nullptr) {
        std::cout<<"can not find codec"<<std::endl;
        return false;
    }
    context = avcodec_alloc_context3(codec);
    if (context == nullptr) {
        std::cout<<"can not allocate decodeContext"<<std::endl;
        return false;
    }
    avcodec_parameters_to_context(context, inputContext->streams[videoStreamIndex]->codecpar);
    int ret = avcodec_open2(context, codec, nullptr);
    if (ret < 0) {
        std::cout<<"can not open decodeContext."<<std::endl;
        char buf[1024]={0};
        av_strerror(ret, buf, 1024);
        std::cout<<std::string(buf);
        return false;
    }
    /* decode frame */
    frame = av_frame_alloc();
    if (frame == nullptr) {
        std::cout<<"failed to allocate frame."<<std::endl;
        return false;
    }
    frame->format = context->pix_fmt;
    frame->width  = context->width;
    frame->height = context->height;
    av_frame_get_buffer(frame, 4);
    /* packet */
    packet = av_packet_alloc();
    if (packet == nullptr) {
        std::cout<<"failed to allocate packet."<<std::endl;
        return false;
    }
    return true;
}

bool Decoder::open()
{
#ifdef WIN64
    AVCodec *codec = nullptr;
#else
    const AVCodec *codec = nullptr;
#endif
    int ret = avcodec_open2(context, codec, nullptr);
    if (ret < 0) {
        std::cout<<"can not open decodeContext."<<std::endl;
        char buf[1024]={0};
        av_strerror(ret, buf, 1024);
        std::cout<<std::string(buf);
        return false;
    }
    /* decode frame */
    frame = av_frame_alloc();
    if (frame == nullptr) {
        std::cout<<"failed to allocate frame."<<std::endl;
        return false;
    }
    frame->format = context->pix_fmt;
    frame->width  = context->width;
    frame->height = context->height;
    av_frame_get_buffer(frame, 4);
    /* packet */
    packet = av_packet_alloc();
    if (packet == nullptr) {
        std::cout<<"failed to allocate packet."<<std::endl;
        return false;
    }
    return true;
}

void Decoder::operator()(std::function<void(AVFrame*)> process)
{
    if (packet->stream_index == videoStreamIndex) {
        /* submit the packet to the decoder */
        int ret = avcodec_send_packet(context, packet);
        if (ret != 0) {
            return;
        }
        while (ret >= 0) {
            ret = avcodec_receive_frame(context, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                break;
            }
            process(frame);
        }
        av_packet_unref(packet);
    }
    return;
}

/*

        decoder.packet->buf = nullptr;
        decoder.packet->pts = frame.startTime();
        decoder.packet->dts = AV_NOPTS_VALUE;
        decoder.packet->data = frame.bits();
        decoder.packet->size = 0;
        decoder.packet->stream_index = 0;
        decoder.packet->flags = 0;
        decoder.packet->side_data = nullptr;
        decoder.packet->side_data_elems = 0;
        decoder.packet->duration = 0;
        decoder.packet->pos = 0;
        decoder.packet->opaque = nullptr;
        decoder.packet->opaque_ref = nullptr;
*/
