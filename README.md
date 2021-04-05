
## 一、子线程记录音频

- 1、找到输入格式 `AVInputFormat：av_find_input_format("avfoundation")` 

- 2、获取上下文：`AVFormatContext *ctx = nullptr`;
- 3、打开设备：`avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr)`;
- 4、QFile 创建文件 `QFile file(filename); 只写模式打开文件：file.open(QFile::WriteOnly)`
- 5、定义数据包 `AVPacket pkt`; 读取数据 `av_read_frame(ctx, &pkt)`;
- 6、写入数据 `file.write((const char *)pkt.data, pkt.size)`;
- 7、关闭文件：`file.close()`; 关闭设备：`avformat_close_input(&ctx)`;
```cpp
extern "C" { //需要的库 (c语言库不能直接在c++中使用需要 extern "C")
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
```
#### 1、 Mac 记录音视频需要申请权限（创建 Info.plist， debug 模式测试）指定路径 QMAKE_INFO_PLIST = xxx/Info.plist
```Objc
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
        <key>NSMicrophoneUsageDescription</key>
        <string>申请使用麦克风</string>
        <key>NSCameraUsageDescription</key>
        <string>申请使用摄像头</string>
</dict>
</plist>
```
#### 2、xxx.pro 配置路径
```
ios {
  FFMPEGHOME = /usr/local/Cellar/ffmpeg/4.3.2_1
}

win32 {
    FFMPEGHOME = ..
}

mac {
    FFMPEGHOME = /usr/local/Cellar/ffmpeg/4.3.2_1

    # Mac 需要 Info.plist 申请音频视频权限，且 debug 才能运行 （avformat_open_input 会 crash）
    # 项目右键 -> Add New -> General -> Empty File -> Info.plist
    # 指定路径 QMAKE_INFO_PLIST = mac/Info.plist
    # （重启QT才生效）
    QMAKE_INFO_PLIST = mac/Info.plist

	# ADD SDL2 HOME
    SDL2_HOME = /usr/local/Cellar/sdl2/2.0.14_1
}

# ADD FFMPEG PATH
INCLUDEPATH += $${FFMPEGHOME}/include

LIBS += -L $${FFMPEGHOME}/lib \
    -lavdevice \
    -lavformat \
    -lavutil

# ADD SDL2 PATH（SDL（Simple DirectMedia Layer），是一个跨平台的C语言多媒体开发库）
INCLUDEPATH += $${SDL2_HOME}/include
LIBS += -L$${SDL2_HOME}/lib \
    -lSDL2
```

#### 3、命令查看支持设备
```shell
~ ffmpeg -devices -hide_banner

Devices:
 D. = Demuxing supported
 .E = Muxing supported
 --
 D  avfoundation    AVFoundation input device
 D  lavfi           Libavfilter virtual input device
  E sdl,sdl2        SDL2 output device
 D  x11grab         X11 screen capture, using XCB
```

```shell
~ ffmpeg -f avfoundation -list_devices true  -i dummy -hide_banner

[AVFoundation indev @ 0x7fa29fe30000] AVFoundation video devices:
[AVFoundation indev @ 0x7fa29fe30000] [0] FaceTime HD Camera
[AVFoundation indev @ 0x7fa29fe30000] [1] Capture screen 0
[AVFoundation indev @ 0x7fa29fe30000] [2] Capture screen 1
[AVFoundation indev @ 0x7fa29fe30000] AVFoundation audio devices:
[AVFoundation indev @ 0x7fa29fe30000] [0] Built-in Microphone
```
## 二、播放 `xx.pcm` 文件
#### 1、使用命令播放 `.pcm`
```shell
~ ffplay -ar 4410 -ac 2 -f s16le Desktop/04_05_12_24_06.pcm
```

- ar: 采样率 (freq, Set the audio sampling frequency.)
- ac: 声道数 (channels, Set the number of audio channels.)
- s16le: PCM signed 16-bit little-endian (有符号、16 位、小段模式)
- 更多PCM的采样格式可以使用命令查看
	- Windows：ffmpeg -formats | findstr PCM
	- Mac：ffmpeg -formats | grep PCM

#### 2、ffplay  基于 ffmpeg 和 SDL 两个库实现
- Mac： /usr/local/Cellar/sdl2（brew install ffmpeg 会安装 SDL2）
- Windows： 使用MinGW编译器，需要自己下SDL2

#### 3、