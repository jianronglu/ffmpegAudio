#include "ffmpegutil.h"
#include <QFile>
#include <QDebug>

FFMPEGUtil::FFMPEGUtil(QObject *parent) : QObject(parent)
{

}

void FFMPEGUtil::pcm_2_wav(WAVHeader &header, const char *scr_pcm_path, const char *dst_wav_path) {

    QFile scr_pcm_file(scr_pcm_path);// 源文件file

    if (!scr_pcm_file.open(QFile::ReadOnly)) {
        qDebug() << "open scr file error:" << scr_pcm_path;
        return;
    }

    header.blockAlign = header.bitsPerSample * header.numChannels >> 3; // 16 * 2 / 8 = 4
    header.byteRate = header.sampleRate * header.blockAlign;    // 44100 * 4

    // 获取 dataChunkData 大小，即pcm文件大小
    header.dataChunkDataSize = scr_pcm_file.size();
    header.riffChunkDataSize = header.dataChunkDataSize
                           + sizeof (WAVHeader)
                           - sizeof (header.riffChunkId)
                           - sizeof (header.riffChunkDataSize);


    QFile dst_wav_file(dst_wav_path); //目标文件

    if (!dst_wav_file.open(QFile::WriteOnly)) {
        scr_pcm_file.close();
        qDebug() << "open dst file error:"<< dst_wav_path;
        return;
    }

    // 写入WAVHeader
    dst_wav_file.write((const char *)&header, sizeof (WAVHeader));

    // 写入 pcm data
    char buf[1024];
    int size;//真正读出来的数据（最后一次读取可能小于1024）
    while ((size = scr_pcm_file.read(buf, sizeof (buf))) > 0) {
        dst_wav_file.write(buf, size);
    }
    // 关闭文件
    scr_pcm_file.close();
    dst_wav_file.close();
}
