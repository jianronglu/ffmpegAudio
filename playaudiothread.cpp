#include "playaudiothread.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

/*
SDL 播放音频有2种模式:
Push(推)：【程序】主动传数据给【音频设备】
Pull(拉)：【音频设备】主动向【程序】拉取数据

ffmpeg -i xxx.mp3 -f s16le -ar 44100 -ac 2 -acodec pcm_s16le pcm16k.pcm ==>把 mp3 转为 pcm
ffplay -ar 44100 -ac 2 -f s16le pcm16k.pcm ==>终端播放 pcm

ffmpeg：强项编解码
SDL2：音视频播放

注意：凡是被压缩过的数据不可以直接播放，需要先解压
*/


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
#define FILE_NAME "/Users/jr.lu/Desktop/pcm16k.pcm"

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

char *bufferData;
int bufferLen;
/*
 * 主拉方式
 * 等待音频设备回调函数（异步====>回到多次）                                               8(bit)->1个字节(bytes)
 * userdata：SDL_AudioSpec.userdata                                1024(样本数)*(16位 * 2声道)/8=4096字节
 * stream：需要往stream填充pcm数据（指向音频缓冲区，大小为 SDL_AudioSpec=>samples * freq * channel)
 * len：希望填充的大小(最后可能不足那么大) => 1024(样本数)*(16位 * 2声道)/8=4096字节
*/
void pull_audio_data(void *userdata, Uint8 * stream,
                                    int len) {
    // 清空stream (用0去填充缓冲区,大小为len)
    SDL_memset(stream, 0, len);

    // 取出缓存数据
    if(bufferLen == 0) return;
    qDebug() << "pull_audio_data bufferLen=" << bufferLen;

    // 取len、bufferLen的最小值（为了保证数据安全，防止指针越界）
    len = (len > bufferLen) ? bufferLen : len;
    // 给音频设备-填充数据，SDL_MIX_MAXVOLUME 软件最大音量不是硬件的
    SDL_MixAudio(stream, (Uint8 *)bufferData, len, SDL_MIX_MAXVOLUME);
    bufferData += len;// 指针往下移动 len (文件读取数据大于缓冲区一次能填充数据量时 eg: data[BUFFER_SIZE * 4]，缓冲区大小为BUFFER_SIZE时, 需要4次填充到缓冲区)
    bufferLen -= len; // SDL_MixAudio 中填充多少减去多少 被消耗数据
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
    } else {
       qDebug() << "文件打开成功" << FILE_NAME;
    }

    // 开始播放 (0->unpause)
    SDL_PauseAudio(0);// 开启这句就会不断走 pull_audio_data 函数

    // 存放文件数据
    char data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        bufferLen = file.read(data, BUFFER_SIZE);

        // 文件数据已经读完
        if(bufferLen <= 0) {
            qDebug() << "数据读完了";
            break;
        }
        qDebug()<< "bufferData 获取到数据";

        // 读取到了文件数据（指向数组首元素）
        bufferData = data;
        while (bufferLen > 0) { // 等待 pull_audio_data 中 SDL_MixAudio 填充完毕
            qDebug() << "SDL_Delay";
            SDL_Delay(1);
        }
    }
    qDebug() << "isInterruptionRequested = true";
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
