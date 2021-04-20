#include "audiothread.h"
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include "ffmpegutil.h"
// ffplay -ar 44100 -ac 2 -f f32le 04_20_10_53_15.pcm  测试读 .pcm 文件

// 查看Mac 设备信息：关于本机->系统报告->音频->设备

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
}


#ifdef Q_OS_MAC
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME ":0"
//    #define FILE_NAME "/Users/jr.lu/Desktop/"
    #define FILE_NAME "/Users/lu/Desktop/"
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
    //qDebug() << "格式 (format):" << params->format; // -1  mac 上不行
    //qDebug() << "一个样本的单声道大小 (format): " << ( av_get_bytes_per_sample((AVSampleFormat)params->format) << 3);
    qDebug() << "格式二 (codec_id)："<< params->codec_id;
    qDebug() <<"一个样本的单声道大小 (codec_id): " << av_get_bits_per_sample(params->codec_id);
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
        char errbuf[1024];
        av_strerror(ret, errbuf, sizeof (errbuf));
        qDebug() << "设备打开失败 ret =" << ret << errbuf;
    } else {
         qDebug() << DEVICE_NAME << "打开设备成功";
    }

    // 打印录音设备的参数信息
    showFormatContext(ctx);

    // 文件名
    QString scr_pcm_filename = FILE_NAME;
    scr_pcm_filename += QDateTime::currentDateTime().toString("MM_dd_HH_mm_ss");
    QString dst_wav_filename = scr_pcm_filename;
    dst_wav_filename += ".wav";
    scr_pcm_filename += ".pcm";
    // 文件操作
    QFile file(scr_pcm_filename);
    //WriteOnly：只写模式。如果文件不存在，就创建文件；如果文件存在，就会清空文件内容
    if(!file.open(QFile::WriteOnly)) {
        qDebug() << "文件打开失败" << scr_pcm_filename;

        // 关闭设备
        avformat_close_input(&ctx);
        return;
    } else {
        qDebug() << "文件打开成功！";
    }

    // 数据包
    AVPacket pkt;
    while (!isInterruptionRequested()) {
        // 不断采集数据
        ret = av_read_frame(ctx, &pkt);
        if (ret == 0) {
            // 写入数据
            qDebug() << "采集数据-成功并写入数据";
            file.write((const char *)pkt.data, pkt.size);
        } else if (ret == AVERROR(EAGAIN)) {//临时资源不可用
            qDebug() << "采集数据-临时资源不可用ret："<< ret;
            continue;
        } else {
            char errbuf[1024];
            av_strerror(ret, errbuf, sizeof (errbuf));
            qDebug() << "采集数据-错误：" << errbuf << ret;
            break;
        }
    }

    // 关闭文件
    file.close();

    // pcm to wav start
    // 获取输入流
    AVStream *stream = ctx->streams[0];
    // 获取音频输入参数
    AVCodecParameters *params = stream->codecpar;

    WAVHeader header;
    header.sampleRate = params->sample_rate;
    header.bitsPerSample = av_get_bits_per_sample(params->codec_id);
    header.numChannels = params->channels;
    if (params->codec_id >= AV_CODEC_ID_PCM_F32BE) {
        header.audioFormat = AUDIO_FORMAT_FLOAT;
    }

    FFMPEGUtil::pcm_2_wav(header,
                          scr_pcm_filename.toUtf8().data(),
                          dst_wav_filename.toUtf8().data());
    // pcm to wav end

    // 关闭设备
    avformat_close_input(&ctx);
}
