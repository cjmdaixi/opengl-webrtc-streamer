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
    av_register_all();
    encoder = new Encoder(true,"output.mp4");
    encoder->Init();
}

void Streamer::Encode(uint8_t *buffer)
{
    encoder->GenOnePkt(buffer);
}

void Streamer::EndStream()
{
    encoder->EndEncode();
}