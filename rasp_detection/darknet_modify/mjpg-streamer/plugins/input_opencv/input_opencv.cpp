/*******************************************************************************
#                                                                              #
# OpenCV input plugin                                                          #
# Copyright (C) 2016 Dustin Spicuzza                                           #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dlfcn.h>
#include <pthread.h>

#include "input_opencv.h"

#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

/* private functions and variables to this plugin */
static globals     *pglobal;

typedef struct {

    int quality_set, quality;

} context_settings;


typedef struct {
    pthread_t   worker;
    
    context_settings *init_settings;
    
} context;


void *worker_thread(void *);
void worker_cleanup(void *);

#define INPUT_PLUGIN_NAME "OpenCV Input plugin"
static char plugin_name[] = INPUT_PLUGIN_NAME;

static context_settings* init_settings() {
    context_settings *settings;
    
    settings = (context_settings*)calloc(1, sizeof(context_settings));
    if (settings == NULL) {
        IPRINT("error allocating context");
        exit(EXIT_FAILURE);
    }
    
    settings->quality = 85;
    return settings;
}

/*** plugin interface functions ***/

/******************************************************************************
Description.: parse input parameters
Input Value.: param contains the command line string and a pointer to globals
Return Value: 0 if everything is ok
******************************************************************************/


int input_init(input_parameter *param, int plugin_no)
{
    int width = 480, height = 360;
    
    input * in;
    context *pctx;
    context_settings *settings;
    
    pctx = new context();
    
    settings = pctx->init_settings = init_settings();
    pglobal = param->global;
    in = &pglobal->in[plugin_no];
    in->context = pctx;

    param->argv[0] = plugin_name;


    IPRINT("Desired Resolution: %i x %i\n", width, height);
    
    return 0;
}

/******************************************************************************
Description.: stops the execution of the worker thread
Input Value.: -
Return Value: 0
******************************************************************************/
int input_stop(int id)
{
    input * in = &pglobal->in[id];
    context *pctx = (context*)in->context;
    
    if (pctx != NULL) {
        DBG("will cancel input thread\n");
        pthread_cancel(pctx->worker);
    }
    return 0;
}

/******************************************************************************
Description.: starts the worker thread and allocates memory
Input Value.: -
Return Value: 0
******************************************************************************/
int input_run(int id)
{
    input * in = &pglobal->in[id];
    context *pctx = (context*)in->context;
    
    in->buf = NULL;
    in->size = 0;
    
    if(pthread_create(&pctx->worker, 0, worker_thread, in) != 0) {
        worker_cleanup(in);
        fprintf(stderr, "could not start worker thread\n");
        exit(EXIT_FAILURE);
    }
    pthread_detach(pctx->worker);

    return 0;
}

void *worker_thread(void *arg)
{
    input * in = (input*)arg;
    context *pctx = (context*)in->context;
    context_settings *settings = (context_settings*)pctx->init_settings;
    
    /* set cleanup handler to cleanup allocated resources */
    pthread_cleanup_push(worker_cleanup, arg);
    
    /* setup imencode options */
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(settings->quality); // 1-100
    
    free(settings);
    pctx->init_settings = NULL;
    settings = NULL;
    
    Mat src, src_resize;
    vector<uchar> jpeg_buffer;
    vector<uchar> jpeg_buffer_broadcast;
    
    // this exists so that the numpy allocator can assign a custom allocator to
    // the mat, so that it doesn't need to copy the data each time
    
    while (!pglobal->stop) {

        /* wait for a net result */
        pthread_mutex_lock(&in->net);
        pthread_cond_wait(&in->net_update, &in->net);

        src = cvarrToMat((IplImage*)in->ipl_net);

        pthread_mutex_unlock(&in->net);

        resize(src,src_resize,Size(640,480),0,0,CV_INTER_LINEAR);

        // TODO: what to do if imencode returns an error?
        imencode(".jpg", src_resize, jpeg_buffer, compression_params);

        /* copy JPG picture to global buffer */
        pthread_mutex_lock(&in->db);

        // take whatever Mat it returns, and write it to jpeg buffer
        jpeg_buffer_broadcast = jpeg_buffer;
        
        // std::vector is guaranteed to be contiguous
        in->buf = &jpeg_buffer_broadcast[0];
        in->size = jpeg_buffer_broadcast.size();
        
        /* signal fresh_frame */
        pthread_cond_broadcast(&in->db_update);
        pthread_mutex_unlock(&in->db);
    }
    
    IPRINT("leaving input thread, calling cleanup function now\n");
    pthread_cleanup_pop(1);

    return NULL;
}

/******************************************************************************
Description.: this functions cleans up allocated resources
Input Value.: arg is unused
Return Value: -
******************************************************************************/
void worker_cleanup(void *arg)
{
    input * in = (input*)arg;
    if (in->context != NULL) {
        context *pctx = (context*)in->context;
        
        delete pctx;
        in->context = NULL;
    }
}
