## srs structure
### app
srs_app_encoder: publish;

srs_app_ffmpeg: transcode engine

srs_app_forward: forward the stream

感觉底下rtc这部分很关键，应该这两天看完。

srs_app_rtc_server: SrsRtcServer listen UDP port, handle UDP packet, manage rtc connections.0

srs_app_rtc_source: SrsRtcSource, a source is a stream to publish and to play with, binding to SrsRtcPublishStream and SrsRtcPlayStream

srs_app_rtc_queue: 这是一个存储结构，存储Udp传过来的包。

srs_app_rtc_api.cpp

层级关系是这样的.
之前的类图很详细，这个网站很赞，但是是3.0版本的，够用。
[srs_note](https://github.com/xialixin/srs_code_note/blob/master/doc/srs_note.md)+
