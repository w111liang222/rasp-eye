#ifndef _CAMH264VIDEOSTREAMFRAMER_H
#define _CAMH264VIDEOSTREAMFRAMER_H

#include <H264VideoStreamFramer.hh>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include "x264_encoder.h"

using namespace cv;

class CamH264VideoStreamFramer: public H264VideoStreamFramer
{
public:
  virtual ~CamH264VideoStreamFramer();
  CamH264VideoStreamFramer(UsageEnvironment& env,FramedSource* inputSource, x264Encoder* pH264Enc);

  static CamH264VideoStreamFramer* createNew(UsageEnvironment& env, FramedSource* inputSource);
  //virtual Boolean currentNALUnitEndsAccessUnit();
  virtual void doGetNextFrame();

private:
  VideoCapture cap;
  x264Encoder *x264;

  x264_nal_t* m_pNalArray;
  int m_iCurNalNum;		//nal数量
  int m_iCurNal;		  //用于计数

};

#endif
