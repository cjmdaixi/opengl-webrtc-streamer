Pixel Streaming

This is a handmade proj following the rules of UE4's pixel-streaming.

### 1 Set Up 
Please make sure ffmpeg is installed first. 
CentOS and macos is recommended.

### 2 Ref
http://dranger.com/ffmpeg/

### Steps
1. Form H.264 from OpenGL buffer.
2. Put them on WebRTC
3. Ans show it in web browser.

### Notes
1. RGB->YUV
   $$
   Y = 0.299R + 0.587B + 0.114*B
   $$

   $$
   U = -0.169R – 0.331G + 0.5 *B
   $$
   
   $$
   V = 0.5 R – 0.419G – 0.081*B
   $$
   
    空域压缩以及时域压缩
    H.264,第一帧为IDR。
   
   |          |                                                              |
   | -------- | ------------------------------------------------------------ |
   | 预测编码 | 预测值与实际值做差，然后再次压缩                             |
   | 帧内预测 | 当前帧不参考其他帧，可以独立解码                             |
   | I帧      | Intra picture 帧内编码图像。IDR是I帧，但是I帧不一定是IDR     |
   | 帧间预测 | 分为B帧和P帧预测                                             |
   | B帧预测  | 参考之前编码的帧和之后编码的帧，但是需要参考后续，引入延时，更多的计算开销 |
   | P帧预测  | 单向预测，参考之前编码过的视频帧                             |
   | 运动估计 | 得到运动矢量的过程就是运动估计                               |
   | 解码视频 | 根据补偿数据，运动数据和参考图像恢复出当前图像。             |
   | DCT      | 离散余弦变换，                                               |


