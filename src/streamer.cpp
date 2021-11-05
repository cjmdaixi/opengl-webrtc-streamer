//
// Created by 宋庭聿 on 2021/10/22.
//

#include "streamer.h"

Streamer::Streamer(Scene & scene):scene_(scene)
{
    encoder = nullptr;
}

void Streamer::SetUpEnv()
{
    SetFFmpeg();
    encoder = new Encoder();
    encoder->Init();
    encoder->InitRtmpPublisher();
}

void Streamer::SetFFmpeg()
{
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
}

void Streamer::Encode(uint8_t *buffer)
{
    encoder->GenOnePkt(buffer);
}

void Streamer::EndStream()
{
    encoder->EndEncode();
}