# rasp-eye
DJI 入职作业

## 说明（基于树莓派的远程智能相机）
树莓派采集视频，由后台GPU服务器进行目标检测。用户打开浏览器访问远程视频及检测结果。

本分支处理树莓派端的视频采集，和H.264流的生成，并实现RTSP服务器推流

### 树莓派:
1) 获取USB摄像头数据，直接采用opencv（基于V4L2）。
2) 视频流编码格式采用H.264，基于x264库进行编码。
3) RTSP推流，可用VLC实现播放
### Ubuntu PC:
1) H.264视频解码。
2) 基于YOLO网络检测视频中的目标。
3) 输出MJPEG视频流，并推送至HTTP服务器。
<div align="center">
  <img src="https://github.com/w111liang222/rasp-eye/blob/master/images/flowchart.jpg"><br><br>
</div>



## 依赖项
### 树莓派:
libcv-dev x264 live555

libcv-dev的安装：sudo apt-get install libcv-dev；
x264的安装：采用源码编译安装，参考https://www.jianshu.com/p/dec9bf9cffc9
live555的编译后请运行sudo make install，自动安装至/usr/local/lib目录下

### Ubuntu PC:
CUDA-8.0 CUDNN OpenBLAS OpenCV3.3.1 CMake FFmpeg

## 编译
```shell
mkdir build
cd build
cmake ..
make
```

## 使用
可执行程序为bin/main
```shell
<<<<<<< HEAD
./rasp_eye_pc -h

Usage: ./rasp_eye_pc image/video [Options]
Options:
  -d            specify the detector to use
                coco
                tiny-coco
                voc
                tiny-voc
  -i            specify the input file
  -w            set image width
  -h            set image height
  -fps          set fps of video
  -g            use gpu index
  -nogpu        don't use gpu
  -thresh       threshold of detector
  -hier
  -h            for help

=======
cd ./build
bin/main
>>>>>>> origin/video_H264_RTSP
```
VLC播放器打开后，点击播放按钮-选择网络选项-输入网络URL：rtsp://192.168.1.102:8554/testStream，即可。

## Reference
https://www.jianshu.com/p/dec9bf9cffc9

## Contact
twei@whu.edu.cn

## License
[GPL-3.0](LICENSE)
