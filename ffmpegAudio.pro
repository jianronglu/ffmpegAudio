QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiothread.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    audiothread.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

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

#DISTFILES += \
#    mac/Info.plist