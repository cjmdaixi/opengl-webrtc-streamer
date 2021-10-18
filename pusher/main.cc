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

int main()
{
    AVOutputFormat *ofmt = nullptr;
    AVFormatContext *ifmt_ctx = nullptr, *ofmt_ctx = nullptr;
    const char* in_filename,*out_filename;
    int ret, i;
    int videoindex=-1;
    int frame_index=0;
    int64_t start_time=0;
    in_filename = "";
    out_filename = "rtmp://localhost/live/livestream";

}