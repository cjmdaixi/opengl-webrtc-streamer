//
// Created by 宋庭聿 on 2021/10/22.
//

#include "streamer.h"

Streamer::Streamer()
{
    encoder = new Encoder(1,"output.mp4");
    rtmp_publisher = nullptr;
}