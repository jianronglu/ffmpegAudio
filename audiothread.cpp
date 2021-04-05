#include "audiothread.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>


extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


#ifdef Q_OS_MAC
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILE_NAME "/Users/jr.lu/Desktop/"
#else
    #define FMT_NAME "dshow"
    #define DEVICE_NAME "audio=xxx"
    #define FILE_NAME "F:/"
#endif

AudioThread::AudioThread(QObject *parent) : QThread(parent)
{
    // 监听线程结束，都调 deleteLabter 方法
    connect(this, &AudioThread::finished, this, &AudioThread::deleteLater);
}

AudioThread::~AudioThread() {
    disconnect(); // 断开连接
    requestInterruption();
    quit();
    wait();
    qDebug() << "AudioThread 析构了";
}

void AudioThread::run(){

    qDebug() << "AudioThread run";
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "input fmt error" << FMT_NAME;
        return;
    } else {
        qDebug() << FMT_NAME << "is ready!";
    }

    // 获取上下文
    AVFormatContext *ctx = nullptr;
    // 打开设备; 0 success; < 0 failure
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, nullptr); // 需要申请权限
    if (ret < 0) {
        char errbuf[1024] = {0};
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "ret =" << ret << errbuf;
    } else {
         qDebug() << DEVICE_NAME << "打开设备成功";
    }

    QString filename = FILE_NAME;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".pcm";
    // 文件操作
    QFile file(filename);

    if(!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;

        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }

    // 数据包
    AVPacket pkt;
    while (!isInterruptionRequested()) {
        ret = av_read_frame(ctx, &pkt);
        if (ret == 0) {
            // 写入数据
            file.write((const char *)pkt.data, pkt.size);
        } else if (ret == AVERROR(EAGAIN)) {//临时资源不可用
            continue;
        } else {
            char errbuf[1024] = {0};
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "av_read_frame error" << errbuf << ret;
            break;
        }
    }

    // 关闭文件
    file.close();

    // 关闭设备
    avformat_close_input(&ctx);
}
