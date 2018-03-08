cmake_minimum_required(VERSION 3.10)

MESSAGE(STATUS "CMAKE_SOURCE_DIR dir:${CMAKE_SOURCE_DIR}")
MESSAGE(STATUS "PATH dir:$ENV{PATH}")
MESSAGE(STATUS "CMAKE_INCLUDE_PATH dir:$ENV{CMAKE_INCLUDE_PATH}")


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ldl -lpthread")
INCLUDE_DIRECTORIES(
${CMAKE_SOURCE_DIR}/src
/usr/local/include/BasicUsageEnvironment
/usr/local/include/groupsock
/usr/local/include/liveMedia
/usr/local/include/UsageEnvironment
)

ADD_EXECUTABLE(main main.cpp x264_encoder.cpp)

LINK_LIBRARIES("/usr/local/lib/libliveMedia.a")
LINK_LIBRARIES("/usr/local/lib/libgroupsock.a")
LINK_LIBRARIES("/usr/local/lib/libBasicUsageEnvironment.a")
LINK_LIBRARIES("/usr/local/lib/libUsageEnvironment.a")



TARGET_LINK_LIBRARIES(main libx264.a)

TARGET_LINK_LIBRARIES(
main
libliveMedia.a
libBasicUsageEnvironment.a
libUsageEnvironment.a
libgroupsock.a
)

TARGET_LINK_LIBRARIES(
main
opencv_core
opencv_imgproc
opencv_video
opencv_highgui
opencv_flann
opencv_ml
opencv_video
opencv_features2d
opencv_calib3d
opencv_objdetect)
