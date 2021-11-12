//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef RTMP_PUBLISHER_H
#define RTMP_PUBLISHER_H

extern "C"{
#include<libavformat/avformat.h>
#include<libavutil/mathematics.h>
#include<libavutil/time.h>
};

#include "common.h"
#include <iostream>
#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER

class RtmpPublisher {
public:
    RtmpPublisher();
    void setUp();
    void publish(uint8_t* buf,int size);
    void endPublish();
private:
    int pts_cnt;
    AVOutputFormat *ofmt;
    AVFormatContext *ofmt_ctx;
    AVStream* out_stream;
    AVCodecParameters  * out_codecpar;
    AVCodec *out_codec ;
    AVCodecContext *out_codec_ctx;
    //AVPacket* pkt;
};


#endif
