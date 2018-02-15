#include "darknet.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

extern void run_detector(int argc, char **argv);

int main(int argc, char **argv)
{

    //TODO Usage Help
    if(argc < 2){
        fprintf(stderr, "usage: %s <function>\n", argv[0]);
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

