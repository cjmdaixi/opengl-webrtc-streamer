//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef STREAMER_H
#define STREAMER_H

#include "encoder.h"
#include "rtmp_publisher.h"

class Streamer {
public:
    Streamer();
    void SetUpEnv();
    void Encode(uint8_t* buffer);
    void OnPublish();
    void DoPublish();
private:
    Encoder* encoder;
    RtmpPublisher* rtmp_publisher;
};


#endif //STREAMER_H
