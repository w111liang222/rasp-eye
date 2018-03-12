#include "CamH264VideoStreamFramer.h"

CamH264VideoStreamFramer::CamH264VideoStreamFramer(UsageEnvironment& env,
    FramedSource* inputSource, x264Encoder* pH264Enc):
      H264VideoStreamFramer(env, inputSource, False, False),
      m_pNalArray(NULL), m_iCurNalNum(0), m_iCurNal(0),x264(pH264Enc)
{
  fFrameRate = 10.0; // We assume a frame rate of 25 fps, unless we learn otherwise (from parsing a Sequence Parameter Set NAL unit)
  //打开摄像头
  cap.open(0);
	cap.set(CV_CAP_PROP_FPS,10);
  if(!cap.isOpened())
  {
		env.setResultMsg("Create camera instance error");
  }
}

CamH264VideoStreamFramer::~CamH264VideoStreamFramer()
{
    //关闭摄像头
    cap.release();
    //销毁H264编码器
    x264->Destory();
    x264 = NULL;
}

CamH264VideoStreamFramer* CamH264VideoStreamFramer::createNew(
                                                         UsageEnvironment& env,
                                                         FramedSource* inputSource)
{

  //初始化H264编码器
  x264Encoder *x264Encoder_tmp = new x264Encoder;
  if(false==x264Encoder_tmp->Create(640,480,3,10))
  {
    env.setResultMsg("Initialize x264 encoder error");
        return NULL;
  }

  CamH264VideoStreamFramer* fr;
  fr = new CamH264VideoStreamFramer(env, inputSource, x264Encoder_tmp);
  return fr;
}

void CamH264VideoStreamFramer::doGetNextFrame()
{
    Mat frame;
    //如果发送完了,获取最新数据帧，存入m_pNalArray
    if(m_iCurNalNum==m_iCurNal)
    {
        m_iCurNal = 0;
		    //从摄像头获取数据
        cap>>frame;
        gettimeofday(&fPresentationTime, NULL);

		    //H264 Encode
        x264->EncodeOneFrame(frame);
        m_iCurNalNum = x264->ReturnNalNum();  //一祯图像数据编码后有多个nal,获得nal数量
        m_pNalArray = x264->ReturnNal();       //获得编码后的nal
    }

    // memmove(fTo, x264->m_encoder->nals[m_iCurNal].p_payload,
    //         pEncode->nals[m_iCurNal].i_payload);

    unsigned char* realData = m_pNalArray[m_iCurNal].p_payload;
    unsigned int realLen = m_pNalArray[m_iCurNal].i_payload;
    m_iCurNal++;

    if(realLen < fMaxSize)
    {
      memmove(fTo, realData, realLen);
    }
    else
    {
      memmove(fTo, realData, fMaxSize);
      fNumTruncatedBytes = realLen - fMaxSize;
    }

    fDurationInMicroseconds = 40000;
    //printf(INF, "fPresentationTime = %d.%d", fPresentationTime.tv_sec, fPresentationTime.tv_usec);

    fFrameSize = realLen;
    afterGetting(this);
}
