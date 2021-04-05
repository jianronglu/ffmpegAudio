#### 子线程记录音频
```c++
extern "C" { //需要的库
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}
```
##### 1、 Mac 记录音视频需要申请权限（创建 Info.plist， debug 模式测试）指定路径 QMAKE_INFO_PLIST = xxx/Info.plist
```cpp
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
##### 2、xxx.pro 配置路径
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
}

INCLUDEPATH += $${FFMPEGHOME}/include

LIBS += -L $${FFMPEGHOME}/lib \
    -lavdevice \
    -lavformat \
    -lavutil
```

##### 3、命令查看支持设备
```zsh
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

```zsh
~ ffmpeg -f avfoundation -list_devices true  -i dummy -hide_banner

[AVFoundation indev @ 0x7fa29fe30000] AVFoundation video devices:
[AVFoundation indev @ 0x7fa29fe30000] [0] FaceTime HD Camera
[AVFoundation indev @ 0x7fa29fe30000] [1] Capture screen 0
[AVFoundation indev @ 0x7fa29fe30000] [2] Capture screen 1
[AVFoundation indev @ 0x7fa29fe30000] AVFoundation audio devices:
[AVFoundation indev @ 0x7fa29fe30000] [0] Built-in Microphone
```
