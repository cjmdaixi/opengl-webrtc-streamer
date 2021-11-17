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

void Streamer::initRtmp() {
    rtmp_publisher = new RtmpPublisher();
    rtmp_publisher->setUp();
}

void Streamer::beginStream()
{
    encoder = new Encoder();
    encoder->Init();
    if(rtmp_publish_option){
        initRtmp();
    }
    else if(rtc_publish_option){
        initRtc();
    }
}

void Streamer::encode(uint8_t *buffer)
{
    int ret_buf_size = 0;
    uint8_t* ret_buf = nullptr;
    encoder->GenOnePkt(buffer,&ret_buf,ret_buf_size);
    if(rtmp_publish_option)
        rtmpPublish(ret_buf,ret_buf_size);
    if(rtc_publish_option)
        rtcPublish(ret_buf,ret_buf_size);
    free(ret_buf); // malloced in encoder
}

void Streamer::rtmpPublish(uint8_t *buf, int size) {
    rtmp_publisher->publish(buf,size);
}

void Streamer::rtcPublish(uint8_t *buf, int size) {
    if(rtc_publisher->connection_setted)
        rtc_publisher->publish(buf,size);
}

void Streamer::endStream()
{
    encoder->EndEncode();
}

void Streamer::initRtc()
{
    rtc_publisher = new RtcPublisher();
    rtc_publisher->setUp();
}
