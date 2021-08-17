//
// Created by 宋庭聿 on 2021/8/18.
// This is a utility
// I create a black picture and convert it to yuv frame
// then I encode them to a h264 vedio stream
// which I can get a packet which store the h264 stream
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
extern "C"{
#include<libavutil/frame.h>
#include<libavutil/mem.h>
#include <libavcodec/avcodec.h>
#include<libavformat/avformat.h>
#include<libavutil/opt.h>
#include<libavutil/imgutils.h>
#include<libswscale/swscale.h>
};

int src_width = 800;
int src_height = 600;
int out_width = 800;
int out_height = 600;

char* outfile = "ds1.mp4";

int main(void){

    FILE* fout = fopen(outfile,"wb+");
    SwsContext* swsContext;
    AVFrame* pframeYUV;
    AVCodec* codec;
    uint8_t *inbuf[2],*outbuf;
    int inlinesize[2] = {src_width*3,0};
    // make red picture
    inbuf[0] = (uint8_t*)malloc(sizeof(uint8_t)*src_height*src_width*3);
    memset(inbuf[0],0,3*src_height*src_width*sizeof(uint8_t));
    inbuf[1] = nullptr;
    // ffmpeg
    av_register_all();
    pframeYUV = av_frame_alloc();
    pframeYUV->format = AV_PIX_FMT_YUV420P;
    pframeYUV->width = out_width;
    pframeYUV->height = out_height;
    avpicture_fill((AVPicture* )pframeYUV,outbuf,AV_PIX_FMT_YUV420P,out_width,out_height);

    // pFrameYUV is set, then we need to convert it to H.264

    codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!codec){
        std::cout<<"avcodec_find_encoder AV_CODEC_ID_HEVC failed"<<std::endl;
    }
    AVCodecContext  *codecCtx = avcodec_alloc_context3(codec);
    if(!codecCtx){
        std::cout<<"avcodec_alloc_context3 failed"<<std::endl;
    }
    codecCtx->bit_rate = 400000;
    codecCtx->width = out_width;
    codecCtx->height = out_height;
    codecCtx->time_base = (AVRational{1,25});
    codecCtx->framerate = (AVRational){25,1};
    codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

    if(codec->id == AV_CODEC_ID_H264)
        av_opt_set(codecCtx->priv_data,"preset","slow",0);
    int ret = avcodec_open2(codecCtx,codec,NULL);
    if(ret<0){
        std::cout<<"avcodec_open2 failed"<<std::endl;
    }

    ret = av_frame_get_buffer(pframeYUV,0);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate the video frame data\n");
        exit(1);
    }
    swsContext = sws_getContext(src_width,src_height,AV_PIX_FMT_RGB24,out_width,out_height,
                                AV_PIX_FMT_YUV420P,SWS_BICUBIC,NULL,NULL,NULL);
    int height = sws_scale(swsContext, (const uint8_t *const*)inbuf, inlinesize,
                           0, src_height,pframeYUV->data, pframeYUV->linesize);
    if(height <= 0) exit(1);
    AVPacket* pkt;
    pkt = av_packet_alloc();
    if(!pkt){
        exit(1);
    }
    // 2 seconds
    for(int i = 0;i < 75;i++){
        pframeYUV->pts = i;
        ret = avcodec_send_frame(codecCtx,pframeYUV);
        if(ret!=0){
            std::cout<<"error sending a frame for encoding\n";
        }
    }
    ret = avcodec_receive_packet(codecCtx,pkt);
    if(ret==0)
        fwrite(pkt->data, 25, pkt->size, fout);

    std::cout<<"<"<<pkt->size<<">";
    avcodec_free_context(&codecCtx);
    av_frame_free(&pframeYUV);
    av_packet_free(&pkt);
    sws_freeContext(swsContext);
    fclose(fout);
    return 0;
}