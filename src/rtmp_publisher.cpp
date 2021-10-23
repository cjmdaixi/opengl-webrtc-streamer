//
// Created by 宋庭聿 on 2021/10/22.
//

#include "rtmp_publisher.h"

RtmpPublisher::RtmpPublisher()
{
    ofmt = nullptr;
    ifmt_ctx = nullptr;
    ofmt_ctx = nullptr;
}

void RtmpPublisher::Init()
{
    avformat_network_init();
}

void RtmpPublisher::Publish(AVPacket* pkt)
{
    // Copy codecpar, the context.

    // Copy packet

}

void RtmpPublisher::EndPublish()
{
    avformat_free_context(ofmt_ctx);
}