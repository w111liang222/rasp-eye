# rasp-eye
DJI 入职作业

## 说明（基于树莓派的远程智能相机）
树莓派采集视频，由后台GPU服务器进行目标检测。用户打开浏览器访问远程视频及检测结果。
本程序分为树莓派与Ubuntu16.04 PC两部分应用程序

### 树莓派:
1) 获取USB摄像头数据，采集接口为V4L2。
2) 视频流编码格式采用H.264，基于FFmpeg库进行编码。
3) 数据传输协议采用TCP协议，保证数据传输的可靠性。
4) 基于ngrok实现内网穿透。
### Ubuntu PC:
1) H.264视频解码。
2) 基于YOLO网络检测视频中的目标。
3) 输出MJPEG视频流，并推送至HTTP服务器。
<div align="center">
  <img src="https://github.com/w111liang222/rasp-eye/blob/master/images/flowchart.png"><br><br>
</div>



## 依赖项
### 树莓派:

### Ubuntu PC:
CUDA-8.0, CUDNN, OpenBLAS, OpenCV3.3.1, CMake, FFmpeg

## 编译
```shell
mkdir build
cd build
cmake ..
make
```

## 使用
Ubuntu PC的可执行程序为rasp_eye_pc
```shell
./rasp_eye_pc -h

Usage: ./rasp_eye_pc image/video \[Options\]
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

```
视频访问:http://127.0.0.1:8080/?action=stream

## Reference
[Darknet](https://pjreddie.com/darknet/)
[MJPEG Streamer](https://github.com/jacksonliam/mjpg-streamer)

## Contact
15lwang@tongji.edu.cn

## License
[GPL-3.0](LICENSE)






