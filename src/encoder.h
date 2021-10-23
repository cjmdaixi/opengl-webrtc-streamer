//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef ENCODER_H
#define ENCODER_H

extern "C"{
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

const int AV_CODEC_ID = AV_CODEC_ID_H264;

#include<iostream>
#include<cstdio>
class Encoder {
public:
    Encoder();
    Encoder(bool write_flag,char* filename);
    void Init();
    void GenOnePkt(uint8_t* buffer);
    void PktToX264();
    void EndEncode();
private:
    uint8_t * in_buf[2];
    bool write_to_file_flag;
    char* out_filename;
    FILE* fout;
private:
    AVCodec* codec;
    AVCodecContext* codecCtx;
    AVPacket* pkt;
    AVFrame* frameYUV;
    SwsContext* swsContext;
};


#endif
