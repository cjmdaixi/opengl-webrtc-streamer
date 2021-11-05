//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef ENCODER_H
#define ENCODER_H

extern "C"{
#include<libavdevice/avdevice.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

#include <mutex>
#include <thread>

const int AV_CODEC_ID = AV_CODEC_ID_H264;

#include<iostream>
#include<cstdio>

#include "rtmp_publisher.h"

class Encoder {
public:
    Encoder();
    void Init();
    void InitRtmpPublisher();
    void GenOnePkt(uint8_t* buffer);
    void DumpLocalVideo();
    void FlushEncoder(int streamIndex);
    void EndEncode();
private:
    RtmpPublisher* rtmpPublisher;
private:
    int frame_count;
    uint8_t * in_buf[2];
    char* out_filename;
    FILE* fout;
private:
    AVCodec* codec;
    AVCodecContext* codecCtx;
    AVFormatContext* ofctx;
    AVStream* stream;
    AVOutputFormat* outputFormat;
    AVPacket* pkt;
    AVFrame* frameYUV;
    SwsContext* swsContext;
// For debug
private:
    void rgb24toppm(uint8_t* buf,int width,int height);
    void yuv420toppm(AVFrame* frame);
    void write_ppm_header(FILE* fp);
};


#endif
