//
// Created by 宋庭聿 on 2021/10/22.
//

#include "rtmp_publisher.h"

RtmpPublisher::RtmpPublisher()
{
    ofmt_ctx = nullptr;
    ofmt = nullptr;
    out_stream = nullptr;
    out_codecpar = nullptr;
    out_codec = nullptr;
    out_codec_ctx = nullptr;
    pts_cnt = 0;
}

void RtmpPublisher::setUp()
{
    int ret = -1;
    // initialize avformat context
     ret = avformat_alloc_output_context2(&ofmt_ctx, nullptr,"flv",nullptr);
    if(ret < 0){
        std::cout<<"Could not allocate output format context"<<std::endl;
        exit(1);
    }
    // initialize io context
    if(!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)){
        int ret = avio_open2(&ofmt_ctx->pb,RTMP_LIVESTEAM,AVIO_FLAG_WRITE, nullptr, nullptr);
        if(ret < 0)
        {
            std::cout<<"Could not open output IO context!"<<std::endl;
            exit(1);
        }
    }
    // set codec params
    out_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    out_stream = avformat_new_stream(ofmt_ctx,out_codec);
    out_codec_ctx = avcodec_alloc_context3(out_codec);

    const AVRational dst_fps = {25,1};
    out_codec_ctx->codec_tag = 0;
    out_codec_ctx->codec_id = AV_CODEC_ID_H264;
    out_codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    out_codec_ctx->width = SCR_WIDTH;
    out_codec_ctx->height = SCR_HEIGHT;
    out_codec_ctx->gop_size = 25;
    out_codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    out_codec_ctx->framerate = dst_fps;
    out_codec_ctx->time_base = av_inv_q(dst_fps);
    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
    {
        out_codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    ret = avcodec_parameters_from_context(out_stream->codecpar, out_codec_ctx);
    if(ret < 0){
        std::cout << "Could not initialize stream codec parameters!" << std::endl;
        exit(1);
    }
    AVDictionary *codec_options = nullptr;
    av_dict_set(&codec_options, "profile", "high", 0);
    av_dict_set(&codec_options, "preset", "superfast", 0);
    av_dict_set(&codec_options, "tune", "zerolatency", 0);
    ret = avcodec_open2(out_codec_ctx, out_codec, &codec_options);
    if (ret < 0)
    {
        std::cout << "Could not open video encoder!" << std::endl;
        exit(1);
    }
    out_stream->codecpar->extradata = out_codec_ctx->extradata;
    out_stream->codecpar->extradata_size = out_codec_ctx->extradata_size;

    av_dump_format(ofmt_ctx,0,RTMP_LIVESTEAM,1);
    ret = avformat_write_header(ofmt_ctx, nullptr);
    if (ret < 0)
    {
        std::cout << "Could not write header!" << std::endl;
        exit(1);
    }
}

void RtmpPublisher::publish(uint8_t* buf,int size)
{
    int cnt =1;
    AVPacket p_pkt;
    int ret = 0,isI = 0;
    int videoIndex = 0;
    out_stream = ofmt_ctx->streams[videoIndex];
    av_init_packet(&p_pkt);
    p_pkt.stream_index = out_stream->index;
    p_pkt.data = buf;
    p_pkt.size = size;
    //p_pkt.size = datalen;
    /* copy packet (remuxing In the example)*/
    AVRational time_base;
    time_base.den = 50;
    time_base.num = 1;
    p_pkt.pts = av_rescale_q((pts_cnt++) * 2, time_base, out_stream->time_base);
    p_pkt.dts = av_rescale_q_rnd(p_pkt.dts, out_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    p_pkt.duration = av_rescale_q(p_pkt.duration, out_stream->time_base, out_stream->time_base);
    p_pkt.pos = -1;

    ret = av_interleaved_write_frame(ofmt_ctx, &p_pkt);
    if (ret < 0) {
        fprintf(stderr, "Error muxing packet\n");
        exit(1);
    }
    printf("write %d frames\n",pts_cnt);

    av_packet_unref(&p_pkt);
}

void RtmpPublisher::endPublish()
{
    avformat_free_context(ofmt_ctx);
}

