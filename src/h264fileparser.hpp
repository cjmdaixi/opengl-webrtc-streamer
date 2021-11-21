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

#ifndef h264fileparser_hpp
#define h264fileparser_hpp

#include <optional>

#include "stream.hpp"

typedef struct
{
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    unsigned max_size;            //! Nal Unit Buffer size
    int forbidden_bit;            //! should be always FALSE
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx
    int nal_unit_type;            //! NALU_TYPE_xxxx
    std::vector<uint8_t> buf;     //! contains the first byte followed by the EBSP
} NALU_t;

class H264FileParser : public StreamSource{
    std::optional<std::vector<std::byte>> previousUnitType5 = std::nullopt;
    std::optional<std::vector<std::byte>> previousUnitType7 = std::nullopt;
    std::optional<std::vector<std::byte>> previousUnitType8 = std::nullopt;
    std::vector<std::uint8_t> preparing_sample;
    bool findStartCode2(std::vector<uint8_t>& buf,int begin);
    bool findStartCode3(std::vector<uint8_t>& buf,int begin);

public:
    void start() override;
    void stop() override;
    const uint64_t sampleDuration_us;
    H264FileParser(uint32_t fps, bool loop);
    void loadNextSample(std::vector<uint8_t>& buffer);
    void clearSentSample();
    void loadNalu(std::vector<uint8_t>& buffer,int begin,int end);
    std::vector<std::byte> initialNALUS();
    bool sample_prepared;
    NALU_t getNalu(std::vector<uint8_t>& buffer,int begin);
};

#endif /* h264fileparser_hpp */
