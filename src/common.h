//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef COMMON_H
#define COMMON_H

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const int inlinesize[2] = {SCR_WIDTH*3,0}; // For sws_scale convert function

const int AV_CODEC_ID = AV_CODEC_ID_H264;

const char* rtmp_fout_name = "rtsp://localhost/livestream/live";

#endif
