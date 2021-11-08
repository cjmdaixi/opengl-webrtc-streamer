//
// Created by 宋庭聿 on 2021/10/22.
//

#include "streamer.h"

Streamer::Streamer(Scene & scene):scene_(scene)
{
    encoder = nullptr;
}

void Streamer::BeginStream()
{
    if(rtmp_publish_option){
        SetFFmpeg();
        encoder = new Encoder();
        encoder->Init();
        encoder->InitRtmpPublisher();
    }
    else if(rtc_publish_option){

    }
}

void Streamer::Encode(uint8_t *buffer)
{
    if(rtmp_publish_option)
        encoder->GenOnePkt(buffer);
}

void Streamer::EndStream()
{
    encoder->EndEncode();
}