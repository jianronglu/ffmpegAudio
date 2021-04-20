#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H

#define AUDIO_FORMAT_PCM 1
#define AUDIO_FORMAT_FLOAT 3

#include <QObject>

typedef struct W {
    // riff chunk (整个文件是riff chunk)
    uint8_t riffChunkId[4] = {'R', 'I', 'F', 'F'}; //RIFF chunk id，4 个字节

    uint32_t riffChunkDataSize; // RIFF chunk的data大小，即文件总长度减去8字节 (需要根据pcm data 计算而来)

    uint8_t format[4] = {'W', 'A', 'V', 'E'}; // format WAVE

    //-------fmt chunk-----
    uint8_t fmtChunkId[4] = {'f', 'm', 't', ' '}; //sub chunk; fmt chunk id

    uint32_t fmtChunkDataSize = 16;// 存PCM时是16

    uint16_t audioFormat = AUDIO_FORMAT_PCM; // 音频编码，1表示PCM，3表示Floating Point

    uint16_t numChannels; // 声道数

    uint32_t sampleRate; //采样率

    uint32_t byteRate;  // 字节率 = sampleRate * blockAlign

    uint16_t blockAlign; //数据块对齐单元 = bitsPerSample * numChannels >> 3（一个样本的字节数）

    uint16_t bitsPerSample;// 位深度

    //------data chunk----
    uint8_t dataChunkId[4] = {'d', 'a', 't', 'a'};//data chunk id

    uint32_t dataChunkDataSize;// 频数据的总长度，即文件总长度减去文件头的长度(一般是44)
} WAVHeader;


class FFMPEGUtil : public QObject {
public:
    explicit FFMPEGUtil(QObject *parent = nullptr);

    /*
     * static 类方法, pcm 格式转 wav
     * header 设置WAV文件参数
     * scr_pcm_path 源pcm文件地址
     * dst_wav_path 目标wav文件地址
     */
    static void pcm_2_wav(WAVHeader &header,
                                const char *scr_pcm_path,
                                const char *dst_wav_path);
signals:

};

#endif // FFMPEGUTIL_H
