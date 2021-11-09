//
// Created by 宋庭聿 on 2021/10/22.
//

#include "streamer.h"

#include <nlohmann/json.hpp>

using namespace rtc;

Streamer::Streamer(Scene & scene):scene_(scene)
{
    encoder = nullptr;
    rtmp_publisher = nullptr;
}

void Streamer::beginStream()
{
    encoder = new Encoder();
    encoder->Init();
    if(rtmp_publish_option){
        encoder->InitRtmpPublisher();
    }
    else if(rtc_publish_option){
        initRtc();
    }
}

void Streamer::encode(uint8_t *buffer)
{
    if(rtmp_publish_option)
        encoder->GenOnePkt(buffer);
}

void Streamer::endStream()
{
    encoder->EndEncode();
}

void Streamer::initRtc()
{
    string stunServer = "stun:stun.l.google.com:19302";
    std::cout<<"Stun server is"<<stunServer<<std::endl;
    rtc_config.iceServers.emplace_back(stunServer);
    rtc_config.disableAutoNegotiation = true;
    std::cout<<"gg"<<std::endl;
}