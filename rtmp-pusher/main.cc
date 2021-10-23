/*
 * Pusher
 * Definition: Pusher is a program using ffmpeg api to push local stream
 * to server to display.
 */

#include <cstdio>

extern "C"
{
#include<libavformat/avformat.h>
#include<libavutil/mathematics.h>
#include<libavutil/time.h>
};

#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)
#define CODEC_FLAG_GLOBAL_HEADER AV_CODEC_FLAG_GLOBAL_HEADER

void exit_av(AVFormatContext * ifmt_ctx,AVFormatContext * ofmt_ctx){
    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ofmt_ctx);
    return;
}

int main()
{
    AVOutputFormat *ofmt = nullptr;
    AVFormatContext *ifmt_ctx = nullptr, *ofmt_ctx = nullptr;
    AVPacket pkt;
    const char* in_filename,*out_filename;
    int ret, i;
    int videoindex=-1;
    int frame_index=0;
    int64_t start_time=0;
    in_filename = "Big_Buck_Bunny_360_10s_1MB.mp4";
    out_filename = "rtmp://localhost/live/livestream";

    av_register_all();
    avformat_network_init();

    if((ret = avformat_open_input(&ifmt_ctx,in_filename,0,0)) < 0){
        printf("Could not open input file");
        exit_av(ifmt_ctx,ofmt_ctx);
    }
    if((ret = avformat_find_stream_info(ifmt_ctx,0)) < 0){
        printf("Failed to retrieve input stream information");
    }

    for(int i = 0; i < ifmt_ctx->nb_streams;i++){
        if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            videoindex = i;
            break;
        }
    }
    av_dump_format(ifmt_ctx,0,in_filename,0);

    avformat_alloc_output_context2(&ofmt_ctx,NULL,"flv",out_filename);
    if(!ofmt_ctx){
        printf("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        exit_av(ifmt_ctx,ofmt_ctx);
    }

    ofmt = ofmt_ctx->oformat;
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        //Create output AVStream according to input AVStream
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            printf( "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            exit_av(ifmt_ctx,ofmt_ctx);
        }
        //Copy the settings of AVCodecContext

        //ret = avcodec_parameters_copy(out_stream->codecpar,ifmt_ctx->streams[i]->codecpar);
        ret = avcodec_copy_context(out_stream->codec,in_stream->codec);
        if (ret < 0) {
            printf( "Failed to copy context from input to output stream codec context\n");
            exit_av(ifmt_ctx,ofmt_ctx);
        }
        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    //Dump Format------------------
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    //Open output URL
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf( "Could not open output URL '%s'", out_filename);
            exit_av(ifmt_ctx,ofmt_ctx);
        }
    }
    //Write file header
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        printf( "Error occurred when opening output URL\n");
        exit_av(ifmt_ctx,ofmt_ctx);
    }

    start_time=av_gettime();
    while (1) {
        AVStream *in_stream, *out_stream;
        //Get an AVPacket
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;
        //FIX：No PTS (Example: Raw H.264)
        //Simple Write PTS
        if(pkt.pts==AV_NOPTS_VALUE){
            //Write PTS
            AVRational time_base1=ifmt_ctx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
            //Parameters
            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt.dts=pkt.pts;
            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
        }
        //Important:Delay
        if(pkt.stream_index==videoindex){
            AVRational time_base=ifmt_ctx->streams[videoindex]->time_base;
            AVRational time_base_q={1,AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);

        }

        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        //Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        //Print to Screen
        if(pkt.stream_index==videoindex){
            printf("Send %8d video frames to output URL\n",frame_index);
            frame_index++;
        }
        //ret = av_write_frame(ofmt_ctx, &pkt);
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

        if (ret < 0) {
            printf( "Error muxing packet\n");
            break;
        }

        av_packet_unref(&pkt);
    }
    //Write file trailer
    av_write_trailer(ofmt_ctx);

    exit_av(ifmt_ctx,ofmt_ctx);
}
