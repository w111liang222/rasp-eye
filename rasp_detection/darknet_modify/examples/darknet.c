#include "darknet.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

extern void run_detector(int argc, char **argv);
static const char usage[]="\
Usage: ./darnet image/video [Options]\n\
Options:\n\
  -d            specify the detector to use\n\
                coco\n\
                tiny-coco\n\
                voc\n\
                tiny-voc\n\
  -i            specify the input file\n\
  -fullscreen   display in fullscreen\n\
  -w            set image width\n\
  -h            set image height\n\
  -fps          set fps of video\n\
  -g            use gpu index\n\
  -nogpu        don't use gpu\n\
  -thresh       threshold of detector\n\
  -hier         \n\
  -h            for help\n\
";
int main(int argc, char **argv)
{
    if(argc<2){
        printf("Please use -h for detail\n");
        return 0;
    }
    if(find_arg(argc,argv, "-h")){
        printf(usage);
        return 0;
    }

    if(0!=strcmp(argv[1], "image") && 0!=strcmp(argv[1], "video")){
        printf("The first para must be 'image' or 'video'\n");
        return 0;
    }

    //determine gpu index
    gpu_index = find_int_arg(argc, argv, "-g", 0);
    if(find_arg(argc, argv, "-nogpu")) {
        gpu_index = -1;
    }

#ifndef GPU
    gpu_index = -1;
#else
    if(gpu_index >= 0){
        cuda_set_device(gpu_index);
    }
#endif

    run_detector(argc, argv);

    return 0;
}

