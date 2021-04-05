#include "playaudiothread.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

// 采样率
#define SAMPLE_RATE 44100
// 采样格式（s16le）
#define SAMPLE_FORMAT AUDIO_S16LSB
// 声道数
#define CHANNELS 2
// 采样大小
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT)
// 音频缓冲区的样本数量
#define SAMPLES 1024
// 每个样本占用多少个字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) / 8)
// 文件缓冲区的大小
#define BUFFER_SIZE (BYTES_PER_SAMPLE * SAMPLES)

// 播放文件路径
#define FILE_NAME "/Users/jr.lu/Desktop/04_05_12_24_06.pcm"

PlayAudioThread::PlayAudioThread(QObject *parent) : QThread(parent)
{
    connect(this, &PlayAudioThread::finished, this, &PlayAudioThread::deleteLater);
}

PlayAudioThread::~PlayAudioThread()
{
    disconnect();
    requestInterruption();
    quit();
    wait();
    qDebug() << "PlayAudioThread 析构了";
}

typedef struct audio_buffer {
    int len = 0;
    int pullLen = 0;
    Uint8 *data = nullptr;
} AudioBuffer;

// userdata：SDL_AudioSpec.userdata
// stream：音频缓冲区（需要将音频数据填充到这个缓冲区）
// len：音频缓冲区的大小（SDL_AudioSpec.samples * 每个样本的大小）
void pull_audio_data(void *userdata, Uint8 * stream,
                                    int len) {
    // 清空stream
    SDL_memset(stream, 0, len);

    // 取出缓存数据
    AudioBuffer *buffer = (AudioBuffer *)userdata;
    if(buffer->len == 0) return;

    // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
    buffer->pullLen = (len > buffer->pullLen) ? buffer->pullLen : len;

    // 填充数据
    SDL_MixAudio(stream, buffer->data, buffer->pullLen, SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;
    buffer->len -= buffer->pullLen;
}

void PlayAudioThread::run()
{
   // 初始化音频系统
   int ret = SDL_Init(SDL_INIT_AUDIO);
   // ret 0 success, < 0 failture
   if (ret) {
       qDebug() << "SDL_init error = " << SDL_GetError();
   }

   // 打开音频设备
   SDL_AudioSpec spec;
   spec.freq = SAMPLE_RATE;
   spec.format = SAMPLE_FORMAT;
   // 音频缓冲区的样本数量 必须是2的幂
   spec.samples = SAMPLES;
   spec.channels = CHANNELS;
   // 传入回调函数
   spec.callback = pull_audio_data;

   //传递给回调的参数
   AudioBuffer buffer;
   spec.userdata = &buffer;

   // 打开音频设备
   if(SDL_OpenAudio(&spec, nullptr)) {
       qDebug()<< "SDL_OpenAudio error:" << SDL_GetError();
       //清除所有的子系统
       SDL_Quit();
       return;
   }

   // 打开文件
   QFile file(FILE_NAME);
   if(!file.open(QFile::ReadOnly)) {
       qDebug() << "打开文件失败:" << FILE_NAME;
       SDL_CloseAudio(); //关闭音频设备
       SDL_Quit(); //清除子系统
       return;
    }

    // 开始播放 (0->unpause)
    SDL_PauseAudio(0);
    // 存放文件数据
    Uint8 data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        // 只要从文件中读取的音频数据，还没有填充完毕，就跳过
        if(buffer.len > 0) continue;

        buffer.len = file.read((char *)data, BUFFER_SIZE);

        // 文件数据已经读完
        if(buffer.len <= 0) {
            //剩余的样本数
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            int ms = samples * 1000 / SAMPLE_RATE;
            SDL_Delay(ms); //延时多少 ms
            break;
        }

        // 读取到了文件数据
        buffer.data = data;
    }

    // 关闭文件
    file.close();
    // 关闭音频设备
    SDL_CloseAudio();
    // 清理所有初始化的子系统
    SDL_Quit();
}

/*
typedef struct SDL_AudioSpec
{
    int freq;                    DSP frequency -- samples per second
    SDL_AudioFormat format;      Audio data format
    Uint8 channels;              Number of channels: 1 mono, 2 stereo
    Uint8 silence;               Audio buffer silence value (calculated)
    Uint16 samples;              Audio buffer size in sample FRAMES (total samples divided by channel count)
    Uint16 padding;              Necessary for some compile environments
    Uint32 size;                 Audio buffer size in bytes (calculated)
    SDL_AudioCallback callback;  Callback that feeds the audio device (NULL to use SDL_QueueAudio()).
    void *userdata;              Userdata passed to callback (ignored for NULL callbacks).
} SDL_AudioSpec;
*/
