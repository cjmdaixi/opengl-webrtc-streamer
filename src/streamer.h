//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef STREAMER_H
#define STREAMER_H

#include "encoder.h"
#include "rtmp_publisher.h"

class Scene;

class Streamer {
public:
    Streamer(Scene & scene);
    void BeginStream();
    void Encode(uint8_t* buffer);
    void SetFFmpeg();
    void SetRtc();
    void EndStream();
private:
    Scene & scene_;
    Encoder* encoder;
    RtmpPublisher* rtmp_publisher;
};


#endif //STREAMER_H
