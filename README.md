Pixel Streaming

The proj is based on cloud rendering.(云渲染)

### 1 Set Up 
Please make sure ffmpeg is installed first. 
CentOS and macos is recommended.

```shell
cmake -B cmake-build-debug
cd cmake-build-debug
make
```

### 2 Funcs

Lots of work still need to be done.

- [x] Record OpenGL app screen and encode to H.264
- [x] Rtmp Streamer, which can push the H.264 raw frame in buffer to server
- [ ] Webrtc
- [ ] HEVC
- [ ] Multi-thread
- [ ] Parallel 
- [ ] Integrate with Irrlicht Game Engine.



