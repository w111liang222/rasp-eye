#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>
#include "x264_encoder.h"

using namespace std;
using std::ofstream;
using namespace cv;

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <GroupsockHelper.hh>
#include <sys/types.h>
#include <sys/stat.h>

UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

// To stream *only* MPEG-1 or 2 video "I" frames
// (e.g., to reduce network bandwidth),
// change the following "False" to "True":
Boolean iFramesOnly = False;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName); // fwd

int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif
  char const* streamName = "H264live";
  //char const* inputFileName = "out.h264";
	char const* inputFileName = "fifo.264";

  //摄像头部分
	uint8_t* buf;
	int bufsize;
	Mat frame1;
	x264Encoder x264;
  VideoCapture cap;

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
					//imshow("FRAME1:",frame1);
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

  // Create the RTSP server:
  RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  char const* descriptionString
    = "Session streamed by \"testOnDemandRTSPServer\"";

  ServerMediaSession* sms
    = ServerMediaSession::createNew(*env, streamName, streamName,
			      descriptionString);
  sms->addSubsession(H264VideoFileServerMediaSubsession
	       ::createNew(*env, inputFileName, reuseFirstSource));
  rtspServer->addServerMediaSession(sms);

  announceStream(rtspServer, sms, streamName, inputFileName);


  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  } else {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName, char const* inputFileName) {
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "\n\"" << streamName << "\" stream, from the file \""
      << inputFileName << "\"\n";
  env << "Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}
