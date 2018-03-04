#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>
#include "x264_encoder.h"

#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
using std::ofstream;
using namespace cv;

UsageEnvironment* env;
char const* inputFileName = "test.264";
H264VideoStreamFramer* videoSource;
RTPSink* videoSink;

void play(); // forward

EventTriggerId DeviceSource::eventTriggerId = 0;
VideoCapture cap;

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  // Create 'groupsocks' for RTP and RTCP:
  struct in_addr destinationAddress;
  destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);
  // Note: This is a multicast address.  If you wish instead to stream
  // using unicast, then you should use the "testOnDemandRTSPServer"
  // test program - not this test program - as a model.

  const unsigned short rtpPortNum = 18888;
  const unsigned short rtcpPortNum = rtpPortNum+1;
  const unsigned char ttl = 255;

  const Port rtpPort(rtpPortNum);
  const Port rtcpPort(rtcpPortNum);

	//摄像头部分
	uint8_t* buf;
	int bufsize;
	Mat frame1;
	x264Encoder x264;

	cap.open(0);
	cap.set(CV_CAP_PROP_FPS,10);
	x264.Create(640,480,3,10);

  mkfifo(inputFileName, 0777);		//创建管道文件，大小需要调整
	pid_t pid = fork();
  if(0 == pid)
  {
		ofstream outfile(inputFileName,ios::out|ios::binary);		//写形式打开，每次写操作定位
    if(!outfile)
    {
        printf("===============child process open pipe err =======\n ");
    }
    while(1)
    {
				//循环获取图像并将264流写入文件中
        usleep(10000);
				if(cap.isOpened())
				{
					cap>>frame1;
					imshow("FRAME1:",frame1);
					bufsize = x264.EncodeOneFrame(frame1);
					if(bufsize>0){
						buf = x264.GetEncodedFrame();
						outfile.write((char *)buf,bufsize);
					}
				}
				else
				{
					cout<<"No camera Input!"<<endl;
					outfile.close();
					return -1;
				}
    }
  }

  Groupsock rtpGroupsock(*env, destinationAddress, rtpPort, ttl);
  rtpGroupsock.multicastSendOnly(); // we're a SSM source
  Groupsock rtcpGroupsock(*env, destinationAddress, rtcpPort, ttl);
  rtcpGroupsock.multicastSendOnly(); // we're a SSM source



  // Create a 'H264 Video RTP' sink from the RTP 'groupsock':
  OutPacketBuffer::maxSize = 600000;
  videoSink = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);

  // Create (and start) a 'RTCP instance' for this RTP sink:
  const unsigned estimatedSessionBandwidth = 10000; // in kbps; for RTCP b/w share
  const unsigned maxCNAMElen = 100;
  unsigned char CNAME[maxCNAMElen+1];
  gethostname((char*)CNAME, maxCNAMElen);
  CNAME[maxCNAMElen] = '\0'; // just in case
  RTCPInstance* rtcp
  = RTCPInstance::createNew(*env, &rtcpGroupsock,
                estimatedSessionBandwidth, CNAME,
                videoSink, NULL /* we're a server */,
                True /* we're a SSM source */);
  // Note: This starts RTCP running automatically

  RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }
  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, "testStream", inputFileName,
           "Session streamed by \"testH264VideoStreamer\"",
                       True /*SSM*/);
  sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, rtcp));
  rtspServer->addServerMediaSession(sms);

  char* url = rtspServer->rtspURL(sms);
  *env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;

  // Start the streaming:
  *env << "Beginning streaming...\n";
  play();

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

void afterPlaying(void* /*clientData*/) {
  *env << "...done reading from file\n";
  videoSink->stopPlaying();
  Medium::close(videoSource);
  cap.release();
  // Note that this also closes the input file that this source read from.

  // Start playing once again:
  play();
}

void play() {
  // Open the input file as a 'byte-stream file source':
  ByteStreamFileSource* fileSource
    = ByteStreamFileSource::createNew(*env, inputFileName);
  if (fileSource == NULL) {
    *env << "Unable to open file \"" << inputFileName
         << "\" as a byte-stream file source\n";
    exit(1);
  }

  FramedSource* videoES = fileSource;

  // Create a framer for the Video Elementary Stream:
  videoSource = H264VideoStreamFramer::createNew(*env, videoES);

  // Finally, start playing:
  *env << "Beginning to read from file...\n";
  videoSink->startPlaying(*videoSource, afterPlaying, videoSink);
}

// int main()
// {
// 	double t;
// 	double fps;
// 	uint8_t* buf;
// 	int bufsize;
// 	int count = 0;
// 	Mat frame1;
// 	x264Encoder x264;
//
// 	ofstream outfile("../out.h264",ios::app|ios::binary);
//
// 	VideoCapture cap(0);
// 	cap.set(CV_CAP_PROP_FPS,10);
// 	x264.Create(640,480,3,10);
//
// 	while(1)
// 	{
// 		if(waitKey(20)==27)break;
// 		//t = (double)getTickCount();
// 		if(cap.isOpened())
// 		{
// 			cap>>frame1;
// 			//t=((double)getTickCount()-t)/getTickFrequency();
// 			//fps = 1.0/t;
// 			//cout<<fps<<endl;
// 			imshow("FRAME1:",frame1);
// 			bufsize = x264.EncodeOneFrame(frame1);
// 			cout<<bufsize<<endl;
// 			if(bufsize>0){
// 				buf = x264.GetEncodedFrame();
// 				outfile.write((char *)buf,bufsize);
// 			}
// 			if(++count>500)break;
// 		}
// 		else
// 		{
// 			cout<<"No camera Input!"<<endl;
// 			outfile.close();
// 			return -1;
// 		}
// 	}
// 	outfile.close();
// 	cout<<"finish~"<<endl;
// 	return 0;
// }
