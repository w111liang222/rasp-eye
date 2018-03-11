#ifndef _H264LIVEVIDEOSERVERMEDIASUBSESSION_H
#define _H264LIVEVIDEOSERVERMEDIASUBSESSION_H


#include "H264VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "FileServerMediaSubsession.hh"
#include "CamH264VideoStreamFramer.h"

class H264LiveVideoServerMediaSubsession: public OnDemandServerMediaSubsession{
public:
  static H264LiveVideoServerMediaSubsession*
  createNew(UsageEnvironment& env, Boolean reuseFirstSource);

private:
  H264LiveVideoServerMediaSubsession(UsageEnvironment& env,
					 Boolean reuseFirstSource);
  // called only by createNew();
  virtual ~H264LiveVideoServerMediaSubsession();

private:
  // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				                    FramedSource* inputSource);
protected:
  virtual char const* sdpLines();

// private:
//   FramedSource * mp_source;
//   char *mp_sdp_line;
//   RTPSink *mp_dummy_rtpsink;
//   int m_done;
};

#endif
