#ifndef _X264_ENCODER_H
#define _X264_ENCODER_H

#include <stdint.h>
#include "x264.h"
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

struct x264_encoder{
    x264_param_t    param;
    char            preset[20];
    char            tune[20];
    char            profile[20];
    x264_t*            h;
    x264_picture_t    pic_in;
    x264_picture_t    pic_out;
    long            colorspace;
    x264_nal_t*        nal;
    int             iframe;
    int             iframe_size;
    int                inal;
};

class x264Encoder
{
public:

    x264Encoder();

    x264Encoder(int videoWidth, int videoHeight, int channel, int fps);

    ~x264Encoder();

    /** 创建X264编码器
     * @param[in] videoWidth  视频宽度
     * @param[in] videoHeight 视频高度
     * @param[in] fps 帧率
     * @return 成功返回true, 失败返回false.
     */
    bool Create(int videoWidth, int videoHeight, int channel = 3, int fps = 30);

    /** 编码一帧
     * @param[in] frame 输入的一帧图像
     * @return 返回编码后数据尺寸, 0表示编码失败
     */
    int EncodeOneFrame(const cv::Mat& frame);

    /** 获取编码后的帧数据
     * 说明: EncodeOneFrame 后调用
     * @return 返回裸x264数据
     */
    uchar* GetEncodedFrame() const;

    /** 销毁X264编码器
     */
    void Destory();

    // 编码器是否可用
    bool IsValid() const;

private:

    void Init();

public:
    int m_width;
    int m_height;
    int m_channel;
    int m_fps;

protected:

    int m_widthstep;
    int m_lumaSize;
    int m_chromaSize;

    x264_encoder*  m_encoder;
};

#endif
