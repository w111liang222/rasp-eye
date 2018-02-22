cmake_minimum_required(VERSION 3.9)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/rasp_detection/darknet_modify/mjpg-streamer/cmake ${CMAKE_MODULE_PATH})
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(FeatureSummary)
include(mjpg_streamer_utils)

project(rasp_eye)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE "Release")



include_directories(/home/lwang/rasp-eye/rasp_detection/darknet_modify/include)
include_directories(/home/lwang/rasp-eye/rasp_detection/darknet_modify/mjpg-streamer)
include_directories(/usr/local/include)

FIND_PACKAGE(CUDA REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED)
find_library(JPEG_LIB jpeg)

ADD_DEFINITIONS(-DOPENCV)
ADD_DEFINITIONS(-DGPU)
ADD_DEFINITIONS(-DCUDNN)

set(GENCODE
        -gencode arch=compute_30,code=sm_30
        -gencode arch=compute_35,code=sm_35
        -gencode arch=compute_50,code=[sm_50,compute_50]
        -gencode arch=compute_52,code=[sm_52,compute_52]
)

set(COMPILE_DEFINITIONS -Werror -Wall)
set(CMAKE_CXX_FLAGS "-Ofast -L/usr/local/cuda/lib64 -L/opt/OpenBLAS/lib")
set(CMAKE_C_FLAGS "-Ofast -L/usr/local/lib -L/usr/local/cuda/lib64 -L/opt/OpenBLAS/lib")
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

#
# Input plugins
#

add_subdirectory(rasp_detection/darknet_modify/mjpg-streamer/plugins/input_http)
add_subdirectory(rasp_detection/darknet_modify/mjpg-streamer/plugins/input_opencv)

#
# Output plugins
#

add_subdirectory(rasp_detection/darknet_modify/mjpg-streamer/plugins/output_http)
add_subdirectory(rasp_detection/darknet_modify/mjpg-streamer/plugins/output_rtsp)

CUDA_ADD_EXECUTABLE(rasp_eye
        rasp_detection/darknet_modify/examples/darknet.c
        rasp_detection/darknet_modify/examples/detector.c
        rasp_detection/darknet_modify/include/darknet.h
        rasp_detection/darknet_modify/src/activation_layer.c
        rasp_detection/darknet_modify/src/activation_layer.h
        rasp_detection/darknet_modify/src/activations.c
        rasp_detection/darknet_modify/src/activations.h
        rasp_detection/darknet_modify/src/avgpool_layer.c
        rasp_detection/darknet_modify/src/avgpool_layer.h
        rasp_detection/darknet_modify/src/batchnorm_layer.c
        rasp_detection/darknet_modify/src/batchnorm_layer.h
        rasp_detection/darknet_modify/src/blas.c
        rasp_detection/darknet_modify/src/blas.h
        rasp_detection/darknet_modify/src/box.c
        rasp_detection/darknet_modify/src/box.h
        rasp_detection/darknet_modify/src/classifier.h
        rasp_detection/darknet_modify/src/col2im.c
        rasp_detection/darknet_modify/src/col2im.h
        rasp_detection/darknet_modify/src/connected_layer.c
        rasp_detection/darknet_modify/src/connected_layer.h
        rasp_detection/darknet_modify/src/convolutional_layer.c
        rasp_detection/darknet_modify/src/convolutional_layer.h
        rasp_detection/darknet_modify/src/cost_layer.c
        rasp_detection/darknet_modify/src/cost_layer.h
        rasp_detection/darknet_modify/src/crnn_layer.c
        rasp_detection/darknet_modify/src/crnn_layer.h
        rasp_detection/darknet_modify/src/crop_layer.c
        rasp_detection/darknet_modify/src/crop_layer.h
        rasp_detection/darknet_modify/src/cuda.c
        rasp_detection/darknet_modify/src/cuda.h
        rasp_detection/darknet_modify/src/data.c
        rasp_detection/darknet_modify/src/data.h
        rasp_detection/darknet_modify/src/deconvolutional_layer.c
        rasp_detection/darknet_modify/src/deconvolutional_layer.h
        rasp_detection/darknet_modify/src/demo.c
        rasp_detection/darknet_modify/src/demo.h
        rasp_detection/darknet_modify/src/detection_layer.c
        rasp_detection/darknet_modify/src/detection_layer.h
        rasp_detection/darknet_modify/src/dropout_layer.c
        rasp_detection/darknet_modify/src/dropout_layer.h
        rasp_detection/darknet_modify/src/gemm.c
        rasp_detection/darknet_modify/src/gemm.h
        rasp_detection/darknet_modify/src/gru_layer.c
        rasp_detection/darknet_modify/src/gru_layer.h
        rasp_detection/darknet_modify/src/im2col.c
        rasp_detection/darknet_modify/src/im2col.h
        rasp_detection/darknet_modify/src/image.c
        rasp_detection/darknet_modify/src/image.h
        rasp_detection/darknet_modify/src/layer.c
        rasp_detection/darknet_modify/src/layer.h
        rasp_detection/darknet_modify/src/list.c
        rasp_detection/darknet_modify/src/list.h
        rasp_detection/darknet_modify/src/local_layer.c
        rasp_detection/darknet_modify/src/local_layer.h
        rasp_detection/darknet_modify/src/lstm_layer.c
        rasp_detection/darknet_modify/src/lstm_layer.h
        rasp_detection/darknet_modify/src/matrix.c
        rasp_detection/darknet_modify/src/matrix.h
        rasp_detection/darknet_modify/src/maxpool_layer.c
        rasp_detection/darknet_modify/src/maxpool_layer.h
        rasp_detection/darknet_modify/src/network.c
        rasp_detection/darknet_modify/src/network.h
        rasp_detection/darknet_modify/src/normalization_layer.c
        rasp_detection/darknet_modify/src/normalization_layer.h
        rasp_detection/darknet_modify/src/option_list.c
        rasp_detection/darknet_modify/src/option_list.h
        rasp_detection/darknet_modify/src/parser.c
        rasp_detection/darknet_modify/src/parser.h
        rasp_detection/darknet_modify/src/region_layer.c
        rasp_detection/darknet_modify/src/region_layer.h
        rasp_detection/darknet_modify/src/reorg_layer.c
        rasp_detection/darknet_modify/src/reorg_layer.h
        rasp_detection/darknet_modify/src/rnn_layer.c
        rasp_detection/darknet_modify/src/rnn_layer.h
        rasp_detection/darknet_modify/src/route_layer.c
        rasp_detection/darknet_modify/src/route_layer.h
        rasp_detection/darknet_modify/src/shortcut_layer.c
        rasp_detection/darknet_modify/src/shortcut_layer.h
        rasp_detection/darknet_modify/src/softmax_layer.c
        rasp_detection/darknet_modify/src/softmax_layer.h
        rasp_detection/darknet_modify/src/stb_image.h
        rasp_detection/darknet_modify/src/stb_image_write.h
        rasp_detection/darknet_modify/src/tree.c
        rasp_detection/darknet_modify/src/tree.h
        rasp_detection/darknet_modify/src/utils.c
        rasp_detection/darknet_modify/src/utils.h
        #CUDA
        rasp_detection/darknet_modify/src/activation_kernels.cu
        rasp_detection/darknet_modify/src/avgpool_layer_kernels.cu
        rasp_detection/darknet_modify/src/blas_kernels.cu
        rasp_detection/darknet_modify/src/col2im_kernels.cu
        rasp_detection/darknet_modify/src/convolutional_kernels.cu
        rasp_detection/darknet_modify/src/crop_layer_kernels.cu
        rasp_detection/darknet_modify/src/deconvolutional_kernels.cu
        rasp_detection/darknet_modify/src/dropout_layer_kernels.cu
        rasp_detection/darknet_modify/src/im2col_kernels.cu
        rasp_detection/darknet_modify/src/maxpool_layer_kernels.cu
        #MJPEG Streamer
        rasp_detection/darknet_modify/mjpg-streamer/mjpg_streamer.c
        rasp_detection/darknet_modify/mjpg-streamer/mjpg_streamer.h
        rasp_detection/darknet_modify/mjpg-streamer/utils.c
        OPTIONS ${GENCODE})

TARGET_LINK_LIBRARIES(rasp_eye openblas)
TARGET_LINK_LIBRARIES(rasp_eye ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(rasp_eye cuda cudart cublas curand cudnn)
