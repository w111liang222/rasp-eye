#include "network.h"
#include "utils.h"
#include <unistd.h>



#define DEMO 1

#ifdef OPENCV

pthread_mutex_t detect_mutex;
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

    image buff_letter = letterbox_image(buff, net->w, net->h);
    image img_process = copy_image(buff);

    double demo_time = what_time_is_it_now();
    while(!demo_done){
        //get newest image
        pthread_mutex_lock(&detect_mutex);
        if(0==frame_done){
            pthread_mutex_unlock(&detect_mutex);
            usleep(1000);
            continue;
        }
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

void demo(char *cfgfile, char *weightfile, float thresh, const char *filename, char **names, int classes, float hier, int w, int h, int frames, int fullscreen, globals* global_ptr)
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

    // Initial Opencv
    CvCapture * cap;
    if(filename){
        printf("Video file: %s\n", filename);
        cap = cvCaptureFromFile(filename);
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
    }else{
        error("No Video Input!\n");
        exit(1);
    }

    //Get Image Size
    double im_origin_width,im_origin_height;
    im_origin_width = cvGetCaptureProperty(cap,CV_CAP_PROP_FRAME_WIDTH);
    im_origin_height = cvGetCaptureProperty(cap,CV_CAP_PROP_FRAME_HEIGHT);
    printf("Input Video Width=%d,Height=%d\n",(int)im_origin_width,(int)im_origin_height);

    cvNamedWindow("Rasp-eye", CV_WINDOW_NORMAL);
    if(fullscreen){
        cvSetWindowProperty("Rasp-eye", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
    } else {
        cvResizeWindow("Rasp-eye", im_origin_width, im_origin_height);
    }

    printf("Load Network...\n");
    net = load_network(cfgfile, weightfile, 0);
    set_batch_network(net, 1);


    buff = get_image_from_stream(cap);
    image img_cap = copy_image(buff);

    //create detect thread
    pthread_t detect_thread;
    if(pthread_create(&detect_thread, 0, detect_in_thread, 0)) error("Thread creation failed");


    double demo_time = what_time_is_it_now();
    while(!demo_done){

        if(0==fill_image_from_stream(cap, img_cap)) break;

        //update buff
        pthread_mutex_lock(&detect_mutex);
        memcpy(buff.data, img_cap.data, img_cap.h*img_cap.w*img_cap.c*sizeof(float));
        frame_done=1;
        pthread_mutex_unlock(&detect_mutex);

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

