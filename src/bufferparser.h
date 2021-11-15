//
// Created by 宋庭聿 on 2021/11/15.
//

#ifndef BUFFERPARSER_H
#define BUFFERPARSER_H

#include "stream.hpp"

class BufferParser : public StreamSource {
public:
    BufferParser(uint32_t samplesPerSecond, bool loop);
    virtual void loadNextSample(uint8_t* buffer,int size);
    virtual void start();
    virtual void stop();
    const uint64_t sampleDuration_us;
private:
    uint32_t frame_counter = -1;
    bool loop;
};


#endif //BUFFERPARSER_H
