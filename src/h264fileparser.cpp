/*
 * libdatachannel streamer example
 * Copyright (c) 2020 Filip Klembara (in2core)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "h264fileparser.hpp"
#include "rtc/rtc.hpp"

#include <fstream>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace std;

H264FileParser::H264FileParser(uint32_t fps, bool loop):sampleDuration_us(1000*1000/fps),StreamSource() {
    sample_prepared = false;
}

void H264FileParser::start(){
    sampleTime_us = std::numeric_limits<uint64_t>::max() - sampleDuration_us + 1;
}

void H264FileParser::stop(){
    StreamSource::stop();
}

void H264FileParser::loadNextSample(std::vector<uint8_t>& buffer) {
    unsigned long long i = 0;
    sampleTime_us += sampleDuration_us;
    sample = *reinterpret_cast<std::vector<std::byte>*>(&buffer);
}

void H264FileParser::loadNalu(std::vector<uint8_t> &buffer, int begin, int end)
{
    std::vector<uint8_t> buf(buffer.begin()+begin,buffer.begin()+end);
    auto tmp = *reinterpret_cast<std::vector<std::byte> *>(&buf);
    sample.insert(sample.end(),tmp.begin(),tmp.end());
}

vector<byte> H264FileParser::initialNALUS() {
    vector<byte> units{};
    if (previousUnitType7.has_value()) {
        auto nalu = previousUnitType7.value();
        units.insert(units.end(), nalu.begin(), nalu.end());
    }
    if (previousUnitType8.has_value()) {
        auto nalu = previousUnitType8.value();
        units.insert(units.end(), nalu.begin(), nalu.end());
    }
    if (previousUnitType5.has_value()) {
        auto nalu = previousUnitType5.value();
        units.insert(units.end(), nalu.begin(), nalu.end());
    }
    return units;
}

void H264FileParser::clearSentSample() {
    sample.clear();
    sample.shrink_to_fit();
}

NALU_t H264FileParser::getNalu(std::vector<uint8_t>& buffer,int begin)
{
    NALU_t nalu;
    bool info2,info3;
    int pos = begin;
    bool nextNaluFound = false;
    info2 = findStartCode2(buffer,begin);
    if(!info2){
        info3 = findStartCode3(buffer,begin);
        if(info3){
            nalu.startcodeprefix_len = 4;
            pos = begin+4;
        }
        else{
            exit(1);
        }
    }
    else{
        nalu.startcodeprefix_len = 3;
        pos = begin+3;
    }
    info3 = false;
    info2 = false;
    while(!nextNaluFound){
        pos++;
        if(pos == buffer.size()){
            nalu.len = (pos - begin) - nalu.startcodeprefix_len;
            nalu.buf.insert(nalu.buf.end(),buffer.begin()+begin+nalu.startcodeprefix_len,buffer.end());
            assert(nalu.buf.size() == nalu.len);
            nalu.forbidden_bit = nalu.buf[0] & 0x80; //1 bit
            nalu.nal_reference_idc = nalu.buf[0] & 0x60; // 2 bit
            nalu.nal_unit_type = (nalu.buf[0]) & 0x1f;// 5 bit
            return nalu;
        }
        info3 = findStartCode3(buffer,pos-4);
        if(!info3){
            info2 = findStartCode2(buffer,pos-3);
        }
        nextNaluFound = info3 || info2;
    }
    int rewind = info3 ? -4:-3;
    nalu.len = (pos - begin + rewind) - nalu.startcodeprefix_len;
    nalu.buf.insert(nalu.buf.end(),buffer.begin()+begin+nalu.startcodeprefix_len,buffer.begin()+begin+nalu.startcodeprefix_len+nalu.len);
    assert(nalu.len == nalu.buf.size());
    nalu.forbidden_bit = nalu.buf[0] & 0x80;
    nalu.nal_reference_idc = nalu.buf[0] & 0x60;
    nalu.nal_unit_type = nalu.buf[0] & 0x1f;
    return nalu;
}

bool H264FileParser::findStartCode2(std::vector<uint8_t>& buf,int begin)
{
    if(buf[begin] == 0x00 && buf[begin+1] == 0x00 && buf[begin+2] == 0x01){
        return true;
    }
    return false;
}
bool H264FileParser::findStartCode3(std::vector<uint8_t>& buf,int begin)
{
    if(buf[begin] == 0x00 && buf[begin+1] == 0x00 && buf[begin+2] == 0x00 && buf[begin+3] == 0x01){
        return true;
    }
    return false;
}
