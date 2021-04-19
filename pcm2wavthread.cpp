#include "pcm2wavthread.h"
#include <QDebug>
#include "ffmpegutil.h"

PCM2WAVThread::PCM2WAVThread(QObject *parent) : QThread(parent)
{
    connect(this, &PCM2WAVThread::finished, this, &PCM2WAVThread::deleteLater);
}

PCM2WAVThread::~PCM2WAVThread(){
    disconnect();
    quit();
    wait();
    requestInterruption();
    qDebug() << "PCM2WAVThread 析构了";
}

void PCM2WAVThread::run() {

    WAVHeader header;
    header.numChannels = 2;
    header.sampleRate = 44100;
    header.bitsPerSample = 16;

    FFMPEGUtil::pcm_2_wav(header, "/Users/lu/Desktop/pcm16k.pcm", "/Users/lu/Desktop/out.wav");

}
