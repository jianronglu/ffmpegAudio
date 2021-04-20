#include "playaudiothread.h"

#include <QDebug>
#include <SDL2/SDL.h>
#include <QFile>

/*
SDL2 播放音频有2种模式:
Push(推)：【程序】主动传数据给【音频设备】
Pull(拉)：【音频设备】主动向【程序】拉取数据

ffmpeg -i xxx.mp3 -f s16le -ar 44100 -ac 2 -acodec pcm_s16le pcm16k.pcm ==>把 mp3 转为 pcm
ffplay -ar 44100 -ac 2 -f s16le pcm16k.pcm ==>终端播放 pcm

ffplay = ffmpeg：强项编解码 + SDL2：音视频播放

注意：凡是被压缩过的数据不可以直接播放，需要先解压


注意如果终端 SougouInput\Components\  ===> 有数据未初始化
*/


// 采样率
#define SAMPLE_RATE 44100
// 采样格式（s16le）
#ifdef Q_OS_MAC
#define SAMPLE_FORMAT AUDIO_F32LSB
#else
#define SAMPLE_FORMAT AUDIO_S16LSB
#endif
// 声道数
#define CHANNELS 2
// 采样大小
#define SAMPLE_SIZE SDL_AUDIO_BITSIZE(SAMPLE_FORMAT) // 根据format & mask 算出位深=>（0x8010 & 0x00FF）
// 音频缓冲区的样本数量
#define SAMPLES 1024
// 每个样本占用多少个字节
#define BYTES_PER_SAMPLE ((SAMPLE_SIZE * CHANNELS) >> 3) // 除8 => >>3
// 文件缓冲区的大小
#define BUFFER_SIZE (BYTES_PER_SAMPLE * SAMPLES)

// 播放文件路径
#define FILE_NAME "/Users/lu/Desktop/04_20_10_53_15.pcm"

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

typedef struct audio_buffer { // c++ 中结构体就是类
    /* 这里不初始化可能会报错，可能用了乱七八糟的栈内存
     * Progam Files(x86) SougouInput\Components\  可能报这个错*/
    int len = 0;  //从文件中读取数据长度
    int pullLen = 0; //实际填充音频设备长度
    char *data = nullptr;//文件读取的数据
} AudioBuffer;

/*
 * 主拉方式
 * 等待音频设备回调函数（异步====>回到多次）                                        8(bit)->1个字节(bytes)
 * userdata：SDL_AudioSpec.userdata                                1024(样本数)*(16位 * 2声道)/8=4096字节
 * stream：需要往stream填充pcm数据（指向音频缓冲区，大小为 SDL_AudioSpec=>samples * freq * channel)
 * len：希望填充的大小(最后可能不足那么大) => 1024(样本数)*(16位 * 2声道)/8=4096字节
*/
void pull_audio_data(void *userdata, Uint8 * stream,
                                    int len) {
    // 取出之前传进来的数据
    AudioBuffer *buffer = (AudioBuffer *)userdata;//结构体指针->访问成员变量

    // 清空stream (用0去填充缓冲区,大小为len)
    SDL_memset(stream, 0, len);
    // 取出缓存数据
    if(buffer->len == 0) return;

    // 取len、buffer->len的最小值（为了保证数据安全，防止指针越界）
    buffer->pullLen = (len > buffer->len) ? buffer->len : len;
    qDebug() << "音频设备索要的大小 =" << len << "文件中实际读取的大小 =" << buffer->len << "实际填充给音频设备大小 =" << buffer->pullLen;
    // 给音频设备-填充buffer->data指向地址开始长度为len数据；SDL_MIX_MAXVOLUME 软件最大音量不是硬件的
    SDL_MixAudio(stream, (Uint8 *)buffer->data, buffer->pullLen, SDL_MIX_MAXVOLUME);
    buffer->data += buffer->pullLen;// 指针往下移动 len (文件读取数据大于缓冲区一次能填充数据量时 eg: data[BUFFER_SIZE * 4]，缓冲区大小为BUFFER_SIZE时, 需要4次填充到缓冲区)
    buffer->len -= buffer->pullLen; // SDL_MixAudio 中填充多少减去多少 被消耗数据
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

   //传递给回调的参数 （局部变量存放在栈内存，注意初始化）
   AudioBuffer buffer; //结构体变量可以直接点语法访问成员变量
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
    SDL_PauseAudio(0);// 开启这句就会不断走 pull_audio_data 回调函数

    // 存放文件数据
    char data[BUFFER_SIZE];
    while (!isInterruptionRequested()) {
        //只要从文件中读取音频数据，还没填充完毕，就跳过
        if (buffer.len > 0) continue;

        buffer.len = file.read(data, BUFFER_SIZE);

        // 文件数据已经读完 （最后一次读数据，pull_audio_data 可能把 buffer.len = 0 了，但是音频设备最后一次数据还没播放完）
        if(buffer.len <= 0) {
            // 推算出还需要多少时间，才能读完剩下数据
            // 剩余的样本数=（填充数据大小 / 每个样本大小）
            int samples = buffer.pullLen / BYTES_PER_SAMPLE;
            //还需要的毫秒数 =（样本数 / 每秒能处理样本-采样率）
            int ms = samples * 1000 / SAMPLE_RATE; //spec.freq = SAMPLE_RATE ;
            SDL_Delay(ms); //毫秒
            qDebug() << "文件数据读完了，音频设备还有:"<<samples<<"个样本没处理完"<<"还需要等待" << ms << "毫秒";
            /*
            *音频设备索要的大小 = 4096 文件中实际读取的大小 = 4096 实际填充给音频设备大小 = 4096
            *文件数据读完了，音频设备还有: 1024 个样本没处理完 还需要等待 23 毫秒
            */
            break;
        }
        qDebug()<< "文件获取到数据大小 ="<< buffer.len;

        // 读取到了文件数据（指向数组首元素）
        buffer.data = data;
    }
    qDebug() << "音频播放完了";
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
