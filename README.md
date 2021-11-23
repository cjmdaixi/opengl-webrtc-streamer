Pixel Streaming

The proj is based on cloud rendering.(云渲染)

### 1 Set Up 
Please make sure ffmpeg is installed first. 
CentOS and macos is recommended.
And please note that the server file is from libdatachannel repo.
```shell
git submodule update --init --recursive
cmake -B cmake-build-debug
cd cmake-build-debug
make
```
before running the program, please make sure the server is set up. 
```shell
cd server
python3 signaling-server.py
python3 -m http.server --bind 127.0.0.1 8080
```
then just run the program, and open http://127.0.0.1:8080/, and press start.

### 2 Funcs

Lots of work still need to be done.

- [x] Record OpenGL app screen and encode to H.264
- [x] Rtmp Streamer, which can push the H.264 raw frame in buffer to server
- [x] Webrtc
- [ ] HEVC
- [x] Multi-thread
- [ ] Parallel 
- [ ] Integrate with Irrlicht Game Engine.

### 3 Next Steps
2021.11.8
- [x] Use librtc(libdatachannel is the origin name) to send memory video.
- [ ] Implement with HEVC

**Please note: the repo's license is MIT, but the 3rd_party/librtc is GPL.**

