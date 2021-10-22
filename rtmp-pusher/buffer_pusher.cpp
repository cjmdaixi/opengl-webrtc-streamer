extern "C"
{
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
};

AVOutputFormat *ofmt = NULL;
AVFormatContext *ofmt_ctx = NULL;
//const char *out_filename = "new.flv";
const char *out_filename = "rtmp://localhost:1985/live/mystream";
int stream_index = 0;
int waitI = 0, rtmpisinit = 0;
int ptsInc = 0;

int GetSpsPpsFromH264(uint8_t* buf, int len)
{
	int i = 0;
	for (i = 0; i < len; i++) {
		if (buf[i+0] == 0x00 
			&& buf[i + 1] == 0x00
			&& buf[i + 2] == 0x00
			&& buf[i + 3] == 0x01
			&& buf[i + 4] == 0x06) {
			break;
		}
	}
	if (i == len) {
		printf("GetSpsPpsFromH264 error...");
		return 0;
	}

	printf("h264(i=%d):", i);
	for (int j = 0; j < i; j++) {
		printf("%x ", buf[j]);
	}
	return i;
}

static bool isIdrFrame2(uint8_t* buf, int len)
{
	switch (buf[0] & 0x1f) {
	case 7: // SPS
		return true;
	case 8: // PPS
		return true;
	case 5:
		return true;
	case 1:
		return false;

	default:
		return false;
		break;
	}
	return false;
}

static bool isIdrFrame1(uint8_t* buf, int size)
{
	int last = 0;
	for (int i = 2; i <= size; ++i) {
		if (i == size) {
			if (last) {
				bool ret = isIdrFrame2(buf + last, i - last);
				if (ret) {
					return true;
				}
			}
		}
		else if (buf[i - 2] == 0x00 && buf[i - 1] == 0x00 && buf[i] == 0x01) {
			if (last) {
				int size = i - last - 3;
				if (buf[i - 3]) ++size;
				bool ret = isIdrFrame2(buf + last, size);
				if (ret) {
					return true;
				}
			}
			last = i + 1;
		}
	}
	return false;
}

//When initializing, the sps and pps data of the first key frame of H264 must be put in.
static int RtmpInit(void* spspps_date, int spspps_datalen)
{
	int ret = 0;
	AVStream *out_stream;
	AVCodecParameters *out_codecpar;
	av_register_all();
	avformat_network_init();

	printf("rtmp init...\n");

	avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", NULL);// out_filename);
	if (!ofmt_ctx) {
		fprintf(stderr, "Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}

	ofmt = ofmt_ctx->oformat;

	out_stream = avformat_new_stream(ofmt_ctx, NULL);
	if (!out_stream) {
		fprintf(stderr, "Failed allocating output stream\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	stream_index = out_stream->index;

	//Because our input is H264 data of a frame read out from memory, we have no input codecpar information, so we have to manually add the output codecpar.
	out_codecpar = out_stream->codecpar;
	out_codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	out_codecpar->codec_id = AV_CODEC_ID_H264;
	out_codecpar->bit_rate = 400000;
	out_codecpar->width = 1280;
	out_codecpar->height = 720;
	out_codecpar->codec_tag = 0;
	out_codecpar->format = AV_PIX_FMT_YUV420P;

	//Extdata (sps and pps data for the first frame of H264) must be added, otherwise FLV with AVCDecoderConfiguration Record information cannot be generated.
	//unsigned char sps_pps[26] = { 0x00, 0x00, 0x01, 0x67, 0x4d, 0x00, 0x1f, 0x9d, 0xa8, 0x14, 0x01, 0x6e, 0x9b, 0x80, 0x80, 0x80, 0x81, 0x00, 0x00, 0x00, 0x01, 0x68, 0xee, 0x3c, 0x80 };
	out_codecpar->extradata_size = spspps_datalen;
	out_codecpar->extradata = (uint8_t*)av_malloc(spspps_datalen + AV_INPUT_BUFFER_PADDING_SIZE);
	if (out_codecpar->extradata == NULL)
	{ 
		printf("could not av_malloc the video params extradata!\n");			
		goto end;
	}
	memcpy(out_codecpar->extradata, spspps_date, spspps_datalen);
	
	av_dump_format(ofmt_ctx, 0, out_filename, 1);

	if (!(ofmt->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
		if (ret < 0) {
			fprintf(stderr, "Could not open output file '%s'", out_filename);
			goto end;
		}
	}

	AVDictionary *opts = NULL;
	av_dict_set(&opts, "flvflags", "add_keyframe_index", 0);
	ret = avformat_write_header(ofmt_ctx, &opts);
	av_dict_free(&opts);
	if (ret < 0) {
		fprintf(stderr, "Error occurred when opening output file\n");
		goto end;
	}

	waitI = 1;
	return 0;

end:
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_closep(&ofmt_ctx->pb);
	if (ofmt_ctx) {
		avformat_free_context(ofmt_ctx);
		ofmt_ctx = NULL;
	}
	return -1;
}

static void VideoWrite(void* data, int datalen)
{
	int ret = 0, isI = 0;
	AVStream *out_stream;
	AVPacket pkt;
	
	out_stream = ofmt_ctx->streams[stream_index];

	av_init_packet(&pkt);
	isI = isIdrFrame1((uint8_t*)data, datalen);
	pkt.flags |= isI ? AV_PKT_FLAG_KEY : 0;
	pkt.stream_index = out_stream->index;
	pkt.data = (uint8_t*)data;
	pkt.size = datalen;
	//wait I frame
	if (waitI) {
		if (0 == (pkt.flags & AV_PKT_FLAG_KEY))
			return;
		else
			waitI = 0;	
	}
	
	AVRational time_base;
	time_base.den = 50;
	time_base.num = 1;
	pkt.pts = av_rescale_q((ptsInc++) * 2, time_base, out_stream->time_base);
	pkt.dts = av_rescale_q_rnd(pkt.dts, out_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt.duration = av_rescale_q(pkt.duration, out_stream->time_base, out_stream->time_base);
	pkt.pos = -1;

	/* copy packet (remuxing In the example)*/
	//pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
	//pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
	//pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
	//pkt.pos = -1;

	ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
	if (ret < 0) {
		fprintf(stderr, "Error muxing packet\n");
	}

	av_packet_unref(&pkt);
}

static void RtmpUnit(void)
{
	if (ofmt_ctx)
		av_write_trailer(ofmt_ctx);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_closep(&ofmt_ctx->pb);
	if (ofmt_ctx) {
		avformat_free_context(ofmt_ctx);
		ofmt_ctx = NULL;
	}
}

int main()
{
	//The following is a simple list of the invocation methods for an excuse. The specific invocation depends on the actual situation.
	
	//preturnps is the input frame 264 data, and iPsLength is the length of a frame
	//Simulated Initialization Call
	char *h264buffer = new char[iPsLength];
	memcpy(h264buffer, preturnps, iPsLength);
	printf("h264 len = %d\n", iPsLength);
	if (!rtmpisinit) {
		if (isIdrFrame1((uint8_t*)h264buffer, iPsLength)) {
				int spspps_len = GetSpsPpsFromH264((uint8_t*)h264buffer, iPsLength);
				if (spspps_len > 0) {
					char *spsbuffer = new char[spspps_len];
					memcpy(spsbuffer, h264buffer, spspps_len);
					rtmpisinit = 1;
					RtmpInit(spsbuffer, spspps_len);
					delete spsbuffer;
				}
		}
	}
	//Start pushing video data
	if (rtmpisinit) {
		VideoWrite(h264buffer, iPsLength);
	}
	//Deinitialization
	RtmpUnit();
	return 0;
}