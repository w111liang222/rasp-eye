#include "H264Decoder.h"
#include <fstream>
#include <linux/videodev2.h>
#include "vlc/libvlc.h"
#include "vlc/libvlc_media.h"
#include "vlc/libvlc_media_player.h"
extern "C" {
#include "camkit.h"
}

using namespace cv;
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


static int VIDEO_WIDTH = 640;
static int VIDEO_HEIGHT = 480;
static char * videobuf = 0;
static char * videobuf_tmp = 0;
IplImage *cap_img = NULL;
pthread_mutex_t cap_mutex;
pthread_cond_t  cap_update;

void *lock(void *data, void**p_pixels)
{
    *p_pixels = videobuf;
    return NULL;
}
void display(void *data, void *id)
{
    pthread_mutex_lock(&cap_mutex);
    memcpy(videobuf_tmp,videobuf,VIDEO_WIDTH * VIDEO_HEIGHT *3);
    cap_img->imageData = videobuf_tmp;
    pthread_cond_broadcast(&cap_update);
    pthread_mutex_unlock(&cap_mutex);
}
void unlock(void *data, void *id, void *const *p_pixels)
{
    (void)data;
    assert(id == NULL);
}

void rtsp_init(const char* url){

    libvlc_media_t* media = NULL;
    static libvlc_media_player_t* mediaPlayer = NULL;
    static libvlc_instance_t* instance = NULL;
    char const* vlc_args[] =
            {
                    "-I",
                    "dummy",
                    ":network-caching=0",
                    "--ignore-config",
            };

    cap_img = cvCreateImage(cvSize(VIDEO_WIDTH, VIDEO_HEIGHT), IPL_DEPTH_8U, 3);
    pthread_mutex_init(&cap_mutex, NULL);


    videobuf = (char*)malloc((VIDEO_WIDTH * VIDEO_HEIGHT) *3);
    videobuf_tmp = (char*)malloc((VIDEO_WIDTH * VIDEO_HEIGHT) *3);
    memset(videobuf, 0, (VIDEO_WIDTH * VIDEO_HEIGHT) *3);

    instance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    media = libvlc_media_new_location(instance, url);
    mediaPlayer = libvlc_media_player_new_from_media(media);
    libvlc_media_release(media);


    libvlc_video_set_callbacks(mediaPlayer, lock, unlock, display, NULL);
    libvlc_video_set_format(mediaPlayer, "RV24", VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH*3);
    libvlc_media_player_play(mediaPlayer);

}

}

