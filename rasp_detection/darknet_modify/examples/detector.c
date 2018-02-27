#include "darknet.h"

void test_detector(char *datacfg, char *cfgfile, char *weightfile, char *filename, float thresh, float hier_thresh)
{
    list *options = read_data_cfg(datacfg);
    char *name_list = option_find_str(options, "names", "data/names.list");
    char **names = get_labels(name_list);

    image **alphabet = load_alphabet();
    network *net = load_network(cfgfile, weightfile, 0);
    set_batch_network(net, 1);
    srand(2222222);
    double time;
    char buff[256];
    char *input = buff;
    int j;
    float nms=.3;
    while(1){
        if(filename){
            strncpy(input, filename, 256);
        } else {
            printf("Enter Image Path: ");
            fflush(stdout);
            input = fgets(input, 256, stdin);
            if(!input) return;
            strtok(input, "\n");
        }
        image im = load_image_color(input,0,0);
        image sized = letterbox_image(im, net->w, net->h);
        //image sized = resize_image(im, net->w, net->h);
        //image sized2 = resize_max(im, net->w);
        //image sized = crop_image(sized2, -((net->w - sized2.w)/2), -((net->h - sized2.h)/2), net->w, net->h);
        //resize_network(net, sized.w, sized.h);
        layer l = net->layers[net->n-1];

        box *boxes = calloc(l.w*l.h*l.n, sizeof(box));
        float **probs = calloc(l.w*l.h*l.n, sizeof(float *));
        for(j = 0; j < l.w*l.h*l.n; ++j) probs[j] = calloc(l.classes + 1, sizeof(float *));
        float **masks = 0;
        if (l.coords > 4){
            masks = calloc(l.w*l.h*l.n, sizeof(float*));
            for(j = 0; j < l.w*l.h*l.n; ++j) masks[j] = calloc(l.coords-4, sizeof(float *));
        }

        float *X = sized.data;
        time=what_time_is_it_now();
        network_predict(net, X);
        printf("%s: Predicted in %f seconds.\n", input, what_time_is_it_now()-time);
        get_region_boxes(l, im.w, im.h, net->w, net->h, thresh, probs, boxes, masks, 0, 0, hier_thresh, 1);
        //if (nms) do_nms_obj(boxes, probs, l.w*l.h*l.n, l.classes, nms);
        if (nms) do_nms_sort(boxes, probs, l.w*l.h*l.n, l.classes, nms);
        draw_detections(im, l.w*l.h*l.n, thresh, boxes, probs, masks, names, alphabet, l.classes);


        save_image(im, "predictions");
#ifdef OPENCV
            cvNamedWindow("predictions", CV_WINDOW_NORMAL);
            show_image(im, "predictions");
            cvWaitKey(0);
            cvDestroyAllWindows();
#endif


        free_image(im);
        free_image(sized);
        free(boxes);
        free_ptrs((void **)probs, l.w*l.h*l.n);
        if (filename) break;
    }
}

void run_detector(int argc, char **argv, globals* global_ptr)
{
    static char detector_data[][50] ={"cfg/coco.data",
                                      "cfg/voc.data"};
    static char detector_cfg[][50]={"cfg/yolo.cfg",
                                    "cfg/tiny-yolo.cfg",
                                    "cfg/yolo-voc.cfg",
                                    "cfg/tiny-yolo-voc.cfg"};
    static char detector_weight[][50]={"yolo.weights",
                                       "tiny-yolo.weights",
                                       "yolo-voc.weights",
                                       "tiny-yolo-voc.weights"};

    float thresh = find_float_arg(argc, argv, "-thresh", .40);
    float hier_thresh = find_float_arg(argc, argv, "-hier", .5);

    int width = find_int_arg(argc, argv, "-w", 0);
    int height = find_int_arg(argc, argv, "-h", 0);
    int fps = find_int_arg(argc, argv, "-fps", 0);

    char *datacfg=NULL;
    char *cfg=NULL;
    char *weights=NULL;

    char *ip_address=NULL;
    int port=0;
    char *filename = find_char_arg(argc, argv, "-i", 0);
    if(NULL==filename){
        printf("Please use '-i' to choose input file, use -h for detail info\n");
        return;
    }

    if(strchr(filename,':')){
        size_t tmp = (size_t)(strchr(filename,':')-filename);
        ip_address = strndup(filename, tmp);//TODO free ip_address
        port = atoi(filename+tmp+1);
        filename = NULL;
    }


    char *detector_type = find_char_arg(argc, argv, "-d", 0);
    if(NULL==detector_type){
        printf("Please use '-d' to choose detector, use -h for detail info\n");
        return;
    }else{
        if(0==strcmp(detector_type, "coco")) {
            datacfg = detector_data[0];
            cfg = detector_cfg[0];
            weights = detector_weight[0];
        }else if(0==strcmp(detector_type, "tiny-coco")) {
            datacfg = detector_data[0];
            cfg = detector_cfg[1];
            weights = detector_weight[1];
        }else if(0==strcmp(detector_type, "voc")) {
            datacfg = detector_data[1];
            cfg = detector_cfg[2];
            weights = detector_weight[2];
        }else if(0==strcmp(detector_type, "tiny-voc")) {
            datacfg = detector_data[1];
            cfg = detector_cfg[3];
            weights = detector_weight[3];
        }else{
            printf("Wrong Detector:%s\n",detector_type);
            return;
        }
    }
    printf("Select Detector:%s\n",detector_type);


    if(0==strcmp(argv[1], "image")) test_detector(datacfg, cfg, weights, filename, thresh, hier_thresh);
    else if(0==strcmp(argv[1], "video")) {
        list *options = read_data_cfg(datacfg);
        int classes = option_find_int(options, "classes", 20);
        char *name_list = option_find_str(options, "names", "data/names.list");
        char **names = get_labels(name_list);
        demo(cfg, weights, thresh, filename, ip_address, port, names, classes, hier_thresh, width, height, fps, global_ptr);
    }else{
        printf("Error option, use -h for detail info\n");
    }

}
