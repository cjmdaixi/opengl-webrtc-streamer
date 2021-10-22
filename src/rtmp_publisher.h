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

#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER

class RtmpPublisher {
public:
    RtmpPublisher();
    void Init();
    void Publish(AVPacket* pkt);
    void EndPublish();
private:

    AVOutputFormat *ofmt;
    AVFormatContext *ifmt_ctx;
    AVFormatContext *ofmt_ctx;
    //AVPacket* pkt;
};


#endif
