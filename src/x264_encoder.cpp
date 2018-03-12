#include "x264_encoder.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <opencv2/imgproc/imgproc.hpp>

#define ENCODER_TUNE   "zerolatency"
#define ENCODER_PROFILE  "baseline"
#define ENCODER_PRESET "veryfast"
#define ENCODER_COLORSPACE X264_CSP_I420
#define CLEAR(x) (memset((&x),0,sizeof(x)))

x264Encoder::x264Encoder()
{
    Init();
}

x264Encoder::x264Encoder(int videoWidth, int videoHeight, int channel, int fps)
{
    Init();
    Create(videoWidth, videoHeight, channel, fps);
}

x264Encoder::~x264Encoder()
{
    Destory();
}

void x264Encoder::Init()
{
    m_width = 0;
    m_height = 0;
    m_channel = 0;
    m_widthstep = 0;
    m_fps = 30;
    m_lumaSize = 0;
    m_chromaSize = 0;
    m_encoder = NULL;
}

bool x264Encoder::Create(int videoWidth, int videoHeight, int channel, int fps)
{
    int ret;
    int imgSize;

    if (videoWidth <= 0 || videoHeight <= 0 || channel < 0 || fps <= 0){
        printf("wrong input param\n");
        return false;
    }
    m_width = videoWidth;
    m_height = videoHeight;
    m_channel = channel;
    m_fps = fps;
    m_widthstep = videoWidth * channel;
    m_lumaSize = m_width * m_height;
    m_chromaSize = m_lumaSize / 4;
    imgSize = m_lumaSize * channel;

    m_encoder = (x264_encoder *)malloc(sizeof(x264_encoder));
    if (!m_encoder){
        printf("cannot malloc x264_encoder !\n");
        return false;
    }
    CLEAR(*m_encoder);
    m_encoder->iframe = 0;
    m_encoder->iframe_size = 0;

    strcpy(m_encoder->preset, ENCODER_PRESET);
    strcpy(m_encoder->tune, ENCODER_TUNE);

    /*初始化编码器*/
    CLEAR(m_encoder->param);
    x264_param_default(&m_encoder->param);

    ret = x264_param_default_preset(&m_encoder->param, m_encoder->preset, m_encoder->tune);
    if (ret < 0){
        printf("x264_param_default_preset error!\n");
        return false;
    }

    /*cpuFlags 去空缓冲区继续使用不死锁保证*/
    m_encoder->param.i_threads = X264_SYNC_LOOKAHEAD_AUTO;
    /*视频选项*/
    m_encoder->param.i_csp = X264_CSP_I420;
    m_encoder->param.i_width = m_width;    // 要编码的图像的宽度
    m_encoder->param.i_height = m_height;    // 要编码的图像的高度
    m_encoder->param.i_frame_total = 0;    // 要编码的总帧数，不知道用0
    m_encoder->param.i_keyint_max = 10*fps;// 关键帧间隔
    /*流参数*/
    m_encoder->param.i_bframe = 5;
    m_encoder->param.b_open_gop = 0;
    m_encoder->param.i_bframe_pyramid = 0;
    m_encoder->param.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;

    /*log参数，不需要打印编码信息时直接注释掉*/
    m_encoder->param.i_log_level = X264_LOG_NONE;

    m_encoder->param.i_fps_num = fps;//码率分子
    m_encoder->param.i_fps_den = 1;    //码率分母

    m_encoder->param.b_intra_refresh = 1;
    m_encoder->param.b_annexb = 1;
    m_encoder->param.rc.f_rf_constant = 24;
    m_encoder->param.rc.i_rc_method = X264_RC_CRF;
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    strcpy(m_encoder->profile, ENCODER_PROFILE);
    ret = x264_param_apply_profile(&m_encoder->param, m_encoder->profile);
    if (ret < 0){
        printf("x264_param_apply_profile error!\n");
        return false;
    }
    /*打开编码器*/
    m_encoder->h = x264_encoder_open(&m_encoder->param);
    m_encoder->colorspace = ENCODER_COLORSPACE;

    /*初始化pic*/
    ret = x264_picture_alloc(&m_encoder->pic_in, m_encoder->colorspace, m_width, m_height);
    if ( ret < 0 ){
        printf("x264_picture_alloc error! ret=%d\n", ret);
        return false;
    }

    m_encoder->pic_in.img.i_csp = m_encoder->colorspace;
    m_encoder->pic_in.img.i_plane = 3;
    m_encoder->pic_in.i_type = X264_TYPE_AUTO;

    m_encoder->inal = 0;
    m_encoder->nal = (x264_nal_t *)calloc(2, sizeof(x264_nal_t));
    if (!m_encoder->nal){
        printf("malloc x264_nal_t error!\n");
        return false;
    }
    CLEAR(*(m_encoder->nal));

    return true;
}

int x264Encoder::EncodeOneFrame(const cv::Mat& frame)
{
    if (frame.empty()){
        return 0;
    }
    cv::Mat bgr(frame), yuv;

    if(1 == frame.channels()){
        cv::cvtColor(frame, bgr, CV_GRAY2BGR);
    }
    cv::cvtColor(bgr, yuv, CV_BGR2YUV_I420);

    memcpy(m_encoder->pic_in.img.plane[0], yuv.data, m_lumaSize);
    memcpy(m_encoder->pic_in.img.plane[1], yuv.data + m_lumaSize, m_chromaSize);
    memcpy(m_encoder->pic_in.img.plane[2], yuv.data + m_lumaSize + m_chromaSize, m_chromaSize);
    m_encoder->pic_in.i_pts = m_encoder->iframe ++;

    //原始码流长度iframe_size
    m_encoder->iframe_size = x264_encoder_encode(m_encoder->h, &m_encoder->nal, &m_encoder->inal, &m_encoder->pic_in, &m_encoder->pic_out);

    return m_encoder->iframe_size;
}

uchar* x264Encoder::GetEncodedFrame() const
{
    return m_encoder->nal->p_payload;
}

void x264Encoder::Destory()
{
    if (m_encoder){
        if (m_encoder->h){
 	        x264_encoder_close(m_encoder->h);
            m_encoder->h = NULL;
        }
        free(m_encoder);
        m_encoder = NULL;
    }
}

bool x264Encoder::IsValid() const
{
    return ((m_encoder != NULL) && (m_encoder->h != NULL));
}

int x264Encoder::ReturnNalNum()
{
  return m_encoder->inal;
}

x264_nal_t* x264Encoder::ReturnNal()
{
  return m_encoder->nal;
}
