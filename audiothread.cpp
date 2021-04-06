#include "audiothread.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>

// ffplay -ar 44100 -ac 2 -f s16le Desktop/04_06_16_11_11.pcm  测试读 .pcm 文件

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}


#ifdef Q_OS_MAC
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":0"
    #define FILE_NAME "/Users/jr.lu/Desktop/"
//    #define FILE_NAME "/Users/lu/Desktop/"
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

void showFormatContext(AVFormatContext *ctx) {
    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取参数
    AVCodecParameters *params = stream->codecpar;
    qDebug() << "采样率:" << params->sample_rate;
    qDebug() << "声道数:" << params->channels;
    // 采样格式(参考枚举类型AVSampleFormat)
    qDebug() << "格式:" << params->format;
    qDebug() << "一个样本的单声道大小" << av_get_bytes_per_sample((AVSampleFormat)params->format);
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

    // 打印录音设备的参数信息
    showFormatContext(ctx);

    // 文件名
    QString filename = FILE_NAME;
    filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    filename += ".pcm";
    // 文件操作
    QFile file(filename);
    //WriteOnly：只写模式。如果文件不存在，就创建文件；如果文件存在，就会清空文件内容
    if(!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << filename;

        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }

    // 数据包
    AVPacket pkt;
    while (!isInterruptionRequested()) {
        // 不断采集数据
        ret = av_read_frame(ctx, &pkt);
        if (ret == 0) {
            // 写入数据
            qDebug() << "写入数据";
            file.write((const char *)pkt.data, pkt.size);
        } else if (ret == AVERROR(EAGAIN)) {//临时资源不可用
            qDebug() << "临时资源不可用ret："<< ret;
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
