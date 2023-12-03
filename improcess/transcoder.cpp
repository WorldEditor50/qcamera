#include "transcoder.h"

Transcoder::Transcoder()
{

}

QImage Transcoder::fromMat(const cv::Mat &src)
{
    QImage img;
    int channel = src.channels();
    switch (channel) {
    case 3:
        img = QImage(src.data, src.cols, src.rows, QImage::Format_RGB888);
        break;
    case 4:
        img = QImage(src.data, src.cols, src.rows, QImage::Format_ARGB32);
        break;
    case 1: {
        img = QImage(src.cols, src.rows, QImage::Format_Indexed8);
        uchar *data = src.data;
        for (int i = 0; i < src.rows ; i++){
            uchar* rowdata = img.scanLine(i);
            for (int j = 0; j < src.cols; j++) {
                rowdata[j] = data[j];
            }
            data += src.cols;
        }
        break;
    }
    default:
        break;
    }
    return img;
}

QImage Transcoder::imageFromVideoFrame(const QVideoFrame& buffer)
{
    QImage img;
    QVideoFrame frame(buffer);  // make a copy we can call map (non-const) on
    frame.map(QAbstractVideoBuffer::ReadOnly);
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(
                frame.pixelFormat());
    // BUT the frame.pixelFormat() is QVideoFrame::Format_Jpeg, and this is
    // mapped to QImage::Format_Invalid by
    // QVideoFrame::imageFormatFromPixelFormat
    if (imageFormat != QImage::Format_Invalid) {
        img = QImage(frame.bits(),
                     frame.width(),
                     frame.height(),
                     // frame.bytesPerLine(),
                     imageFormat);
    } else {
        // e.g. JPEG
        int nbytes = frame.mappedBytes();
        img = QImage::fromData(frame.bits(), nbytes);
    }
    frame.unmap();
    return img;
}


int Transcoder::videoFrameToRGBA(QVideoFrame &frame, int &h, int &w, unsigned char *rgba)
{
    frame.map(QAbstractVideoBuffer::ReadOnly);
    w = frame.width();
    h = frame.height();
    int ret = -1;
    if (frame.pixelFormat() == QVideoFrame::Format_NV21) {
        unsigned char *y = frame.bits();
        unsigned char *uv = frame.bits() + h*w;
        ret = libyuv::NV21ToARGB(y, w, uv, w, rgba, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_NV12) {
        unsigned char *y = frame.bits();
        unsigned char *uv = frame.bits() + h*w;
        ret = libyuv::NV12ToARGB(y, w, uv, w, rgba, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_YUYV) {
        int alignedWidth = (w + 1) & ~1;
        ret = libyuv::YUY2ToARGB(frame.bits(), alignedWidth*2, rgba, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_YUV420P) {
        unsigned char *y = frame.bits();
        unsigned char *u = frame.bits() + h*w;
        unsigned char *v = frame.bits() + h*w*5/4;
        ret = libyuv::I420ToARGB(y, w, u, w/2, v, w/2, rgba, w*4, w, h);
    }
    frame.unmap();
#ifdef Q_OS_ANDROID
    cv::Mat img(h, w, CV_8UC4, rgba);
    cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
#endif
    return ret;
}

int Transcoder::videoFrameToMat(QVideoFrame &frame, cv::Mat &img)
{
    frame.map(QAbstractVideoBuffer::ReadOnly);
    int w = frame.width();
    int h = frame.height();
    img = cv::Mat(h, w, CV_8UC4);
    int ret = -1;
    if (frame.pixelFormat() == QVideoFrame::Format_NV21) {
        unsigned char *y = frame.bits();
        unsigned char *uv = frame.bits() + h*w;
        ret = libyuv::NV21ToARGB(y, w, uv, w, img.data, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_NV12) {
        unsigned char *y = frame.bits();
        unsigned char *uv = frame.bits() + h*w;
        ret = libyuv::NV12ToARGB(y, w, uv, w, img.data, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_YUYV) {
        int alignedWidth = (w + 1) & ~1;
        ret = libyuv::YUY2ToARGB(frame.bits(), alignedWidth*2, img.data, w*4, w, h);
    } else if (frame.pixelFormat() == QVideoFrame::Format_YUV420P) {
        unsigned char *y = frame.bits();
        unsigned char *u = frame.bits() + h*w;
        unsigned char *v = frame.bits() + h*w*5/4;
        ret = libyuv::I420ToARGB(y, w, u, w/2, v, w/2, img.data, w*4, w, h);
    }
    frame.unmap();
    cv::cvtColor(img, img, cv::COLOR_RGBA2BGR);
#ifdef Q_OS_ANDROID
    cv::rotate(img, img, cv::ROTATE_90_CLOCKWISE);
#endif
    return ret;
}

void Transcoder::nv21ToRGB24(unsigned char *yuyv, unsigned char *rgb, int width, int height)
{
    const int nv_start = width * height ;
    int  index = 0, rgb_index = 0;
    unsigned char y, u, v;
    int r, g, b, nv_index = 0,i, j;

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j ++){
            //nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
            nv_index = i / 2  * width + j - j % 2;

            y = yuyv[rgb_index];
            u = yuyv[nv_start + nv_index ];
            v = yuyv[nv_start + nv_index + 1];

            r = y + (140 * (v-128))/100;  //r
            g = y - (34 * (u-128))/100 - (71 * (v-128))/100; //g
            b = y + (177 * (u-128))/100; //b

            if(r > 255)   r = 255;
            if(g > 255)   g = 255;
            if(b > 255)   b = 255;
            if(r < 0)     r = 0;
            if(g < 0)     g = 0;
            if(b < 0)     b = 0;

            index = rgb_index % width + (height - i - 1) * width;
            //rgb[index * 3+0] = b;
            //rgb[index * 3+1] = g;
            //rgb[index * 3+2] = r;

            //rgb[height * width * 3 - i * width * 3 - 3 * j - 1] = b;
            //rgb[height * width * 3 - i * width * 3 - 3 * j - 2] = g;
            //rgb[height * width * 3 - i * width * 3 - 3 * j - 3] = r;

            rgb[i * width * 3 + 3 * j + 0] = b;
            rgb[i * width * 3 + 3 * j + 1] = g;
            rgb[i * width * 3 + 3 * j + 2] = r;

            rgb_index++;
        }
    }
    return;
}

void Transcoder::yuv420pToRGB24(unsigned char *yuv420p, unsigned char *rgb24, int width, int height)
{
    int index = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int indexY = y * width + x;
            int indexU = width * height + y / 2 * width / 2 + x / 2;
            int indexV = width * height + width * height / 4 + y / 2 * width / 2 + x / 2;

            unsigned char Y = yuv420p[indexY];
            unsigned char U = yuv420p[indexU];
            unsigned char V = yuv420p[indexV];

            rgb24[index++] = Y + 1.402 * (V - 128); //R
            rgb24[index++] = Y - 0.34413 * (U - 128) - 0.71414 * (V - 128); //G
            rgb24[index++] = Y + 1.772 * (U - 128); //B
        }
    }
    return;
}

void Transcoder::yuyvToRGB24(unsigned char *yuv_buffer,unsigned char *rgb_buffer,int iWidth,int iHeight)
{
    int x;
    int z=0;
    unsigned char *ptr = rgb_buffer;
    unsigned char *yuyv= yuv_buffer;
    for (x = 0; x < iWidth*iHeight; x++)
    {
        int r, g, b;
        int y, u, v;

        if (!z)
            y = yuyv[0] << 8;
        else
            y = yuyv[2] << 8;
        u = yuyv[1] - 128;
        v = yuyv[3] - 128;

        r = (y + (359 * v)) >> 8;
        g = (y - (88 * u) - (183 * v)) >> 8;
        b = (y + (454 * u)) >> 8;

        *(ptr++) = (r > 255) ? 255 : ((r < 0) ? 0 : r);
        *(ptr++) = (g > 255) ? 255 : ((g < 0) ? 0 : g);
        *(ptr++) = (b > 255) ? 255 : ((b < 0) ? 0 : b);

        if(z++)
        {
            z = 0;
            yuyv += 4;
        }
    }
    return;
}


