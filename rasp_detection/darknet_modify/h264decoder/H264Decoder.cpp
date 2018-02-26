//
// Created by curi on 17-6-28.
//

#include "H264Decoder.h"

void H264Decoder::init() {

    matReady = false;

    avcodec_register_all();
    av_init_packet(&avpkt);

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /*if (codec->capabilities & AV_CODEC_CAP_TRUNCATED){
        c->flags |= AV_CODEC_CAP_TRUNCATED;
    }*/

    //初始化解析器
    cp = av_parser_init(AV_CODEC_ID_H264);
    if (!cp){
        printf("Could not alloc parser failed!\n");
        exit(1);
    }

    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    //存储解码后转换的RGB数据
    pFrameBGR = av_frame_alloc();


}





void H264Decoder::decode(unsigned char *inputbuf, size_t insize){

    if(insize<=0)   return;

    if(size<=0){
        buf = new unsigned char[insize];
        memcpy(buf,inputbuf,sizeof(unsigned char)*insize);
        size = insize;
    }else{
        unsigned char* tmp_buf = new unsigned char[size];
        memcpy(tmp_buf,buf,sizeof(unsigned char)*size);
        free(buf);
        buf = nullptr;
        buf = new unsigned char[size+insize];
        memcpy(buf,tmp_buf,sizeof(unsigned char)*size);
        memcpy(buf+size,inputbuf,sizeof(unsigned char)*insize);
        free(tmp_buf);
        size = size+insize;
    }

    int len = av_parser_parse2(cp, c,
                           &avpkt.data, &avpkt.size, buf, size,
                           AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

    if(avpkt.size == 0) return;



    int ret, got_frame;
    ret = avcodec_decode_video2(c, frame, &got_frame, &avpkt);


    size -=len;
    if(size<=0){
        free(buf);
        buf = nullptr;
    }else{
        unsigned char* tmp_buf = new unsigned char[size];
        memcpy(tmp_buf,buf+len,sizeof(unsigned char)*size);
        free(buf);
        buf = nullptr;
        buf = new unsigned char[size];
        memcpy(buf,tmp_buf,sizeof(unsigned char)*size);
        free(tmp_buf);
    }

    if (ret < 0) {
        matReady = false;
        fprintf(stderr, "Error while decoding frame\n");
        return ;
    }
    if(out_buffer == nullptr){
        BGRsize = avpicture_get_size(AV_PIX_FMT_BGR24, c->width, c->height);
        out_buffer = (uint8_t *) av_malloc(BGRsize);
        avpicture_fill((AVPicture *) pFrameBGR, out_buffer, AV_PIX_FMT_BGR24, c->width, c->height);

        img_convert_ctx = sws_getContext(c->width, c->height, c->pix_fmt,
                               c->width, c->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
        pCvMat.create(cv::Size(c->width, c->height), CV_8UC3);

    }
    if (got_frame) {
        matReady = true;
        sws_scale(img_convert_ctx, (const uint8_t *const *)frame->data,
                  frame->linesize, 0, c->height, pFrameBGR->data, pFrameBGR->linesize);

        memcpy(pCvMat.data, out_buffer, BGRsize);

    }
    else{
        matReady = false;
    }


}

void H264Decoder::play() {
    if(matReady){
        cv::imshow("Raw-Image",pCvMat);
        cv::waitKey(1);
    }
}

H264Decoder::H264Decoder() {
    init();
}
H264Decoder::~H264Decoder() {
    if(nullptr!=c)  av_free(c);
}

cv::Mat H264Decoder::getMat() {
    if(matReady){
        return pCvMat;
    }
    else{
        return cv::Mat();
    }
}

void H264Decoder::getIpl(IplImage ** ipl){
    if(matReady) {
        pIpl = IplImage(pCvMat);
        *ipl = &pIpl;
    }
}



