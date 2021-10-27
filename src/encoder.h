//
// Created by 宋庭聿 on 2021/10/22.
//

#ifndef ENCODER_H
#define ENCODER_H

extern "C"{
#include<libavdevice/avdevice.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};

#include <mutex>
#include <thread>

const int AV_CODEC_ID = AV_CODEC_ID_H264;

#include<iostream>
#include<cstdio>

#define BITS_PER_PIXCEL 24
#define FORMAT_RGB 3

typedef unsigned char  	BYTE;
typedef unsigned short  WORD;
typedef unsigned int  	DWORD;
typedef int  		LONG;

typedef struct
{
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BMP_FILE_HEADER;

typedef struct {
    DWORD biSize;
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BMP_INFO_HEADER;

class Encoder {
public:
    Encoder();
    Encoder(bool write_flag,char* filename);
    void Init();
    void GenOnePkt(uint8_t* buffer);
    void PktToX264();
    void FlushEncoder(int streamIndex);
    void EndEncode();
private:
    void rgb24toppm(uint8_t* buf,int width,int height);
    void yuv420toppm(AVFrame* frame);
    void write_ppm_header(FILE* fp);
private:
    int frame_count;
    uint8_t * in_buf[2];
    bool write_to_file_flag;
    char* out_filename;
    FILE* fout;
private:
    AVCodec* codec;
    AVCodecContext* codecCtx;
    AVFormatContext* ofctx;
    AVStream* stream;
    AVOutputFormat* outputFormat;
    AVPacket* pkt;
    AVFrame* frameYUV;
    SwsContext* swsContext;
};


#endif
