QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiothread.cpp \
    ffmpegutil.cpp \
    main.cpp \
    mainwindow.cpp \
    pcm2wavthread.cpp \
    playaudiothread.cpp

HEADERS += \
    audiothread.h \
    ffmpegutil.h \
    mainwindow.h \
    pcm2wavthread.h \
    playaudiothread.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    FFMPEGHOME = ..
    SDL2_HOME = ..
}

mac {
    # FFMPEG HOME
#    FFMPEGHOME = /usr/local/Cellar/ffmpeg/4.3.2_1
    FFMPEGHOME = /usr/local/Cellar/ffmpeg/4.3.2_3 #版本不同路径不同
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

LIBS += -L$${FFMPEGHOME}/lib \  # -L：去什么路径查找
    -lavdevice \ # -l 连接xxx库
    -lavformat \
    -lavutil \
    -lavcodec

# ADD SDL2 PATH
INCLUDEPATH += $${SDL2_HOME}/include
LIBS += -L$${SDL2_HOME}/lib \
    -lSDL2
