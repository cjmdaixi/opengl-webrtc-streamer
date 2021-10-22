//
// Created by 宋庭聿 on 2021/10/22.
//

#include "encoder.h"

#include "common.h"

Encoder::Encoder(){
    codec = nullptr;
    codecCtx = nullptr;
    pkt = nullptr;
    frameYUV = nullptr;
    swsContext = nullptr;
    write_to_file_flag = false;
    out_filename = nullptr;
}

Encoder::Encoder(bool write_flag,char* filename){
    codec = nullptr;
    codecCtx = nullptr;
    pkt = nullptr;
    frameYUV = nullptr;
    swsContext = nullptr;
    write_to_file_flag = write_flag;
    out_filename = filename;
    fout = fopen(out_filename,"wb");
}

void Encoder::Init()
{
    // AV_CODEC_ID is declared in common.h
    codec = avcodec_find_encoder((AVCodecID) AV_CODEC_ID);
    codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx){
        std::cout<<"avcodec_alloc_context3 failed"<<std::endl;
    }
    pkt = av_packet_alloc();
    if(!pkt)
        exit(1);

    codecCtx->bit_rate = 400000;
    codecCtx->width = SCR_WIDTH;
    codecCtx->height = SCR_HEIGHT;
    codecCtx->time_base = (AVRational){1,25};
    codecCtx->framerate = (AVRational){25,1};
    codecCtx->gop_size = 10;
    codecCtx->max_b_frames = 1;
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    if(codec->id == AV_CODEC_ID_H264){
        av_opt_set(codecCtx->priv_data,"preset","slow",0);
    }
    int ret = avcodec_open2(codecCtx,codec,NULL);
    if(ret < 0){
        std::cout<<"avcodec_open2 failed"<<std::endl;
    }

    frameYUV = av_frame_alloc();
    if(!frameYUV){
        std::cerr<<"Could not allocate video frame\n";
        exit(1);
    }
    frameYUV->format = AV_PIX_FMT_YUV420P;
    frameYUV->width = SCR_WIDTH;
    frameYUV->height = SCR_HEIGHT;
    ret = av_frame_get_buffer(frameYUV,0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }

    swsContext = sws_getContext(SCR_WIDTH,SCR_HEIGHT,AV_PIX_FMT_RGB24,
                                SCR_WIDTH,SCR_HEIGHT,AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC,NULL,NULL,NULL);
}

void Encoder::EndEncode()
{
    avcodec_free_context(&codecCtx);
    av_frame_free(&frameYUV);
    av_packet_free(&pkt);
    sws_freeContext(swsContext);
}

void Encoder::GenOnePkt(uint8_t* buffer)
{
    // TODO: memory leak may happen here
    memcpy(in_buf[0],buffer,sizeof(uint8_t)*SCR_HEIGHT*SCR_WIDTH);
    in_buf[1] = nullptr;
    int height = sws_scale(swsContext,(const uint8_t* const*)in_buf,inlinesize,0,SCR_HEIGHT,
                           frameYUV->data,frameYUV->linesize);
    if(height <= 0) exit(1);
    // TODO: whether pts info needed should be further discuss
    // pframeYUV->pts = count++ * (codecCtx->time_base.num * 1000 / codecCtx->time_base.den);
    int ret = avcodec_send_frame(codecCtx,frameYUV);
    if(ret < 0){
        printf("Error sending a frame for encoding");
        exit(1);
    }
    while(ret >= 0){
        ret = avcodec_receive_packet(codecCtx,pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return;
        }
        else if(ret < 0){
            std::cerr<<"error during encoding"<<std::endl;
            exit(1);
        }
        if(write_to_file_flag){
            PktToX264();
        }
        // TODO: send this pkt data to rtsp out file.
        av_packet_unref(pkt);
    }
}

void Encoder::PktToX264()
{
    fwrite(pkt->data,1,pkt->size,fout);
}