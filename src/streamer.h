//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef STREAMER_H
#define STREAMER_H

#include <rtc/rtc.hpp>
#include <rtc/websocket.hpp>
#include <nlohmann/json.hpp>
#include "encoder.h"
#include "rtmp_publisher.h"
#include "rtc_publisher.h"

class Scene;

using json = nlohmann::json;

class Streamer {
public:
    Streamer(Scene & scene);
    void beginStream();
    void endStream();
    void encode(uint8_t* buffer);
private:
    Scene & scene_;
    Encoder* encoder;
    RtmpPublisher* rtmp_publisher;
    RtcPublisher* rtc_publisher;
private:
    void initRtc();
    void initRtmp();
    void rtmpPublish(uint8_t* buf,int size);
    void rtcPublish(uint8_t* buf,int size);
};


#endif //STREAMER_H
