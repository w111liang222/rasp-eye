#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <iostream>
#include <fstream>
#include "x264_encoder.h"

using namespace std;
using std::ofstream;
using namespace cv;

int main()
{
	double t;
	double fps;
	uint8_t* buf;
	int bufsize;
	int count = 0;
	Mat frame1;
	x264Encoder x264;

	ofstream outfile("../out.h264",ios::app|ios::binary);

	VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FPS,10);
	x264.Create(640,480,3,10);

	while(1)
	{
		if(waitKey(20)==27)break;
		//t = (double)getTickCount();
		if(cap.isOpened())
		{
			cap>>frame1;
			//t=((double)getTickCount()-t)/getTickFrequency();
			//fps = 1.0/t;
			//cout<<fps<<endl;
			imshow("FRAME1:",frame1);
			bufsize = x264.EncodeOneFrame(frame1);
			cout<<bufsize<<endl;
			if(bufsize>0){
				buf = x264.GetEncodedFrame();
				outfile.write((char *)buf,bufsize);
			}
			if(++count>500)break;
		}
		else
		{
			cout<<"No camera Input!"<<endl;
			outfile.close();
			return -1;
		}
	}
	outfile.close();
	cout<<"finish~"<<endl;
	return 0;
}
