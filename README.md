# GstreamerPlayer
安装和使用说明见博客 [Gstreamer安装和使用](https://yadiq.github.io/2022/08/15/MediaGstreamerInstall/)

#### 介绍
1. gstreamer播放器。
tutorial 1-4，为官方教程代码。
app，为自己实现的播放器，可以执行Gstreamer Pipline。

2. 生成的动态库如下：
编译音频 libgstreamer_audio.so
编译视频 libgstreamer_video.so
编译gstreamer底层 libgstreamer_android.so、libc++_shared.so

#### 待完善
1. 将gstreamer audio 和 video 合并到 player