#include "network.h"
#include "utils.h"
#include "layer.h"
#include <unistd.h>

extern void get_remote_image(IplImage ** ipl, const char* ip_address, const int port);

#define DEMO 1

#ifdef OPENCV

pthread_mutex_t detect_mutex;
extern IplImage *cap_img;
extern pthread_mutex_t cap_mutex;
extern pthread_cond_t  cap_update;
int frame_done=0;

static globals* demo_global_ptr;
static char **demo_names;
static image **demo_alphabet;
static int demo_classes;

static float **probs;
static box *boxes;
static network *net;
static image buff;

static double p_fps,v_fps;
static float demo_thresh = 0.24,demo_hier = .5;
static int demo_frame = 0;
static int demo_detections = 0;
static int demo_done = 0;

struct net_config{
    char *cfg;
    char *weight;
};

void *display_in_thread(void *ptr)
{

    image *im_ptr= (image*)ptr;
    draw_detections(*im_ptr, demo_detections, demo_thresh, boxes, probs, 0, demo_names, demo_alphabet, demo_classes);


    IplImage  * ipl = cvCreateImage(cvSize(im_ptr->w,im_ptr->h), IPL_DEPTH_8U, im_ptr->c);
    if(NULL==demo_global_ptr->in[0].ipl_net){
        demo_global_ptr->in[0].ipl_net = cvCreateImage(cvSize(im_ptr->w,im_ptr->h), IPL_DEPTH_8U, im_ptr->c);
    }
    //copy to global buffer
    pthread_mutex_lock(&demo_global_ptr->in[0].net);

    cvCopy(ipl,(IplImage  *)demo_global_ptr->in[0].ipl_net,NULL);

    pthread_cond_broadcast(&demo_global_ptr->in[0].net_update);
    pthread_mutex_unlock(&demo_global_ptr->in[0].net);

    show_image_cv(*im_ptr, "Rasp-eye", ipl);
    cvShowImage("Rasp-eye", ipl);

    cvWaitKey(1);

    cvReleaseImage(&ipl);
    return 0;
}

void *detect_in_thread(void *ptr)
{
    const float nms = .4;

    layer l = net->layers[net->n-1];
    //variable initial
    float **predictions = calloc(demo_frame, sizeof(float*));
    for(int j = 0; j < demo_frame; ++j){
        predictions[j] = (float *) calloc(l.outputs, sizeof(float));
    }

    probs = (float **)calloc(l.w*l.h*l.n, sizeof(float *));
    for(int j = 0; j < l.w*l.h*l.n; ++j){
        probs[j] = (float *)calloc(l.classes+1, sizeof(float));
    }

    float *avg = (float *) calloc(l.outputs, sizeof(float));
    boxes = (box *)calloc(l.w*l.h*l.n, sizeof(box));
    demo_detections = l.n*l.w*l.h;

    image buff_letter,img_process;
    while(1){
        pthread_mutex_lock(&detect_mutex);

        if(0==frame_done){
            pthread_mutex_unlock(&detect_mutex);
            usleep(5000);
            continue;
        }
        buff_letter = letterbox_image(buff, net->w, net->h);
        img_process = copy_image(buff);
        cvNamedWindow("Rasp-eye", CV_WINDOW_NORMAL);
        cvResizeWindow("Rasp-eye", buff.w, buff.h);

        pthread_mutex_unlock(&detect_mutex);
        break;
    }

    double demo_time = what_time_is_it_now();
    while(!demo_done){
        //get newest image
        pthread_mutex_lock(&detect_mutex);

        memcpy(img_process.data, buff.data, buff.h*buff.w*buff.c*sizeof(float));
        frame_done=0;
        pthread_mutex_unlock(&detect_mutex);

        static int demo_index = -1;
        demo_index = (demo_index + 1)%demo_frame;


        letterbox_image_into(img_process, net->w, net->h, buff_letter);
        float *prediction = network_predict(net, buff_letter.data);

        memcpy(predictions[demo_index], prediction, l.outputs*sizeof(float));
        mean_arrays(predictions, demo_frame, l.outputs, avg);
        l.output = avg;
        if(l.type == DETECTION){
            get_detection_boxes(l, 1, 1, demo_thresh, probs, boxes, 0);
        } else if (l.type == REGION){
            get_region_boxes(l, img_process.w, img_process.h, net->w, net->h, demo_thresh, probs, boxes, 0, 0, 0, demo_hier, 1);
        } else {
            error("Last layer must produce detections\n");
        }
        if (nms > 0) do_nms_obj(boxes, probs, l.w*l.h*l.n, l.classes, nms);

        //waiting for display finish
        static pthread_t disp_thread;
        pthread_join(disp_thread,NULL);

        p_fps = 1./(what_time_is_it_now() - demo_time);
        demo_time = what_time_is_it_now();
        printf("\033[2J");
        printf("\033[1;1H");
        printf("\nFPS:%.1f\n",p_fps);
        printf("vFPS:%.1f\n",v_fps);
        printf("Objects:\n\n");

        //display results
        if(pthread_create(&disp_thread, 0, display_in_thread, &img_process)) error("Thread creation failed");

    }

    return 0;
}

void *net_loading_in_thread(void *ptr){
    struct net_config *net_ptr = (struct net_config *)ptr;

    printf("Load Network...\n");
    net = load_network(net_ptr->cfg, net_ptr->weight, 0);
    set_batch_network(net, 1);

    //create detect thread
    pthread_t detect_thread;
    if(pthread_create(&detect_thread, 0, detect_in_thread, 0)) error("Thread creation failed");
    return NULL;
}

void demo(char *cfgfile, char *weightfile, float thresh, const char *filename, char *ip_address, int port, char *rtsp_url, char **names, int classes, float hier, int w, int h, int frames, globals* global_ptr)
{

    demo_global_ptr = global_ptr;
    pthread_mutex_init(&detect_mutex, NULL);



    demo_frame = 3;//average the prediction results
    demo_names = names;
    demo_alphabet = load_alphabet();
    demo_classes = classes;
    demo_thresh = thresh;
    demo_hier = hier;

    srand(2222222);

    struct net_config net_conf;
    net_conf.cfg = cfgfile;
    net_conf.weight = weightfile;
    pthread_t net_loading_thread;
    if(pthread_create(&net_loading_thread, 0, net_loading_in_thread, &net_conf)) error("Thread creation failed");

    // Initial Capture
    CvCapture * cap;
    if(filename){
        printf("Video file: %s\n", filename);
        cap = cvCreateFileCapture(filename);
        if(!cap) {
            fprintf(stderr,"Couldn't connect to video %s.\n",filename);
            exit(1);
        }
        if(w){
            cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_WIDTH, w);
        }
        if(h){
            cvSetCaptureProperty(cap, CV_CAP_PROP_FRAME_HEIGHT, h);
        }
        if(frames){
            cvSetCaptureProperty(cap, CV_CAP_PROP_FPS, frames);
        }

    }else if(ip_address){
        printf("Remote video from %s:%d\n",ip_address,port);
    }else if(rtsp_url){
        printf("RTSP stream from %s\n",rtsp_url);
    }

    //capture fisrt image for initialize
    pthread_mutex_lock(&detect_mutex);

    if(filename){
        buff = get_image_from_stream(cap);
    }else if(ip_address){
        //remote image capture
        while(1){
            IplImage* src = NULL;
            get_remote_image(&src, ip_address, port);
            if(NULL==src){
                printf("Waiting for full image complete!\n");
                continue;
            }
            buff = ipl_to_image(src);
            rgbgr_image(buff);
            break;
        }
    }else if(rtsp_url){
        pthread_mutex_lock(&cap_mutex);
        pthread_cond_wait(&cap_update, &cap_mutex);

        buff = ipl_to_image(cap_img);
        rgbgr_image(buff);

        pthread_mutex_unlock(&cap_mutex);
    }

    frame_done=1;
    pthread_mutex_unlock(&detect_mutex);

    printf("Input Video Width=%d,Height=%d\n",buff.w,buff.h);

    image img_cap = copy_image(buff);


    double demo_time = what_time_is_it_now();
    while(!demo_done){

        IplImage *src = NULL;

        if(filename){
            src = cvQueryFrame(cap);
            if (NULL==src) continue;
            ipl_into_image(src, img_cap);

        }else if(ip_address){
            //remote image capture
            get_remote_image(&src, ip_address, port);
            if(NULL==src)   continue;
            ipl_into_image(src, img_cap);

        }else if(rtsp_url){
            pthread_mutex_lock(&cap_mutex);
            pthread_cond_wait(&cap_update, &cap_mutex);

            src = cap_img;
            ipl_into_image(src, img_cap);

            pthread_mutex_unlock(&cap_mutex);
        }

        //
        rgbgr_image(img_cap);

        //update buff
        pthread_mutex_lock(&detect_mutex);
        memcpy(buff.data, img_cap.data, img_cap.h*img_cap.w*img_cap.c*sizeof(float));
        frame_done=1;
        pthread_mutex_unlock(&detect_mutex);

        cvShowImage("Raw", src);
        cvWaitKey(1);

        v_fps = 1./(what_time_is_it_now() - demo_time);
        demo_time = what_time_is_it_now();

    }

}


#else
void demo(char *cfgfile, char *weightfile, float thresh, const char *filename, char **names, int classes, float hier, int w, int h, int frames, int fullscreen)
{
    fprintf(stderr, "Demo needs OpenCV for webcam images.\n");
}
#endif

