//
// Created by 宋庭聿 on 2021/11/15.
//

#include "bufferparser.h"

BufferParser::BufferParser(uint32_t samplesPerSecond, bool loop) :sampleDuration_us(1000*1000/samplesPerSecond),StreamSource(){
    this->loop = loop;
}

void BufferParser::start()
{
    sampleTime_us = std::numeric_limits<uint64_t>::max() - sampleDuration_us + 1;
}

void BufferParser::stop() {
    StreamSource::stop();
    frame_counter = -1;
}

void BufferParser::loadNextSample(uint8_t* buffer,int size) {
    std::string frame_id = std::to_string(++frame_counter);
    std::vector<uint8_t> buf(buffer,buffer+size); //
    sample = *reinterpret_cast<std::vector<std::byte> *>(&buf);
}

