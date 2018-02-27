#include "H264Decoder.h"
#include <fstream>
#include <linux/videodev2.h>
extern "C" {
#include "camkit.h"
}

using namespace std;

extern "C" {

//#define USE_CAM_KIT

#ifdef USE_CAM_KIT
int WIDTH=640,HEIGHT=480;

struct cap_handle *caphandle = NULL;
struct cvt_handle *cvthandle = NULL;
struct enc_handle *enchandle = NULL;


struct cap_param capp;
struct cvt_param cvtp;
struct enc_param encp;


void *cap_buf, *cvt_buf;
int cap_len, cvt_len;
enum pic_t ptype;
void camkit_init(void){

    static bool flag=false;
    if(flag)  return;

    flag=true;
    capp.dev_name = "/dev/video1";
    capp.width = WIDTH;
    capp.height = HEIGHT;
    capp.pixfmt = V4L2_PIX_FMT_YUYV;
    capp.rate = 30;

    cvtp.inwidth = WIDTH;
    cvtp.inheight = HEIGHT;
    cvtp.inpixfmt = V4L2_PIX_FMT_YUYV;
    cvtp.outwidth = WIDTH;
    cvtp.outheight = HEIGHT;
    cvtp.outpixfmt = V4L2_PIX_FMT_YUV420;

    encp.src_picwidth = WIDTH;
    encp.src_picheight = HEIGHT;
    encp.enc_picwidth = WIDTH;
    encp.enc_picheight = HEIGHT;
    encp.chroma_interleave = 0;
    encp.fps = 30;
    encp.gop = 30;
    encp.bitrate = 1600;


    caphandle = capture_open(capp);
    if (!caphandle)
    {
        printf("--- Open capture failed\n");
        return ;
    }

    cvthandle = convert_open(cvtp);
    if (!cvthandle)
    {
        printf("--- Open convert failed\n");
        return ;
    }

    enchandle = encode_open(encp);
    if (!enchandle)
    {
        printf("--- Open encode failed\n");
        return ;
    }


    capture_start(caphandle);		// !!! need to start capture stream!

}

void get_buffer(void **buf,int *size){
    int ret = capture_get_data(caphandle, &cap_buf, &cap_len);
    if (ret != 0) {
        if (ret < 0)        // error
        {
            printf("--- capture_get_data failed\n");
        } else    // again
        {
            usleep(10000);
        }
    }
    if (cap_len <= 0) {
        printf("!!! No capture data\n");
    }

    ret = convert_do(cvthandle, cap_buf, cap_len, &cvt_buf, &cvt_len);
    if (ret < 0)
    {
        printf("--- convert_do failed\n");
    }
    if (cvt_len <= 0)
    {
        printf("!!! No convert data\n");
    }

    ret = encode_do(enchandle, cvt_buf, cvt_len, buf, size,
                    &ptype);
    if (ret < 0)
    {
        printf("--- encode_do failed\n");
    }
    if (*size <= 0)
    {
        printf("!!! No encode data\n");
    }
}
#endif

void get_remote_image(IplImage ** ipl, const char* ip_address, const int port) {

    void *buf;
    int len = 0;
    static H264Decoder decoder;

    //receive buffer from tcp
#ifdef USE_CAM_KIT
    camkit_init();
    get_buffer(&buf,&len);
#endif

    decoder.decode((unsigned char*)buf, len);
    //decoder.play();
    decoder.getIpl(ipl);

    return;
}


}

