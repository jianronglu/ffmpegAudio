#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
AudioThread *thread = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 记录音频
void MainWindow::on_recodeAudioBtn_clicked()
{
    if (!_audioThread) {
       _audioThread = new AudioThread(this);
       _audioThread->start();//会自动调用run

       connect(_audioThread, &AudioThread::finished, [this] () {//线程结束 (lanmbda 表达式 oc 中 block)
          _audioThread = nullptr;// 必须捕获 this 不然无法访问 _audioThread
          ui->recodeAudioBtn->setText("开始录音");
       });
       qDebug() << "AudioThread Start";
       ui->recodeAudioBtn->setText("结束录音");
    } else {
       _audioThread->requestInterruption();
    }
}

// 播放音频
void MainWindow::on_playAudioBtn_clicked()
{
    if(!_playAudioThread) {
        _playAudioThread = new PlayAudioThread(this);
        _playAudioThread->start();

        connect(_playAudioThread, &PlayAudioThread::finished, [this]() {
            _playAudioThread = nullptr;
            ui->playAudioBtn->setText("播放音乐");
        });
        qDebug()<< "PlayAudioThread Start";
        ui->playAudioBtn->setText("停止播放");
     } else {
        _playAudioThread->requestInterruption();
        qDebug()<< "PlayAudioThread end";
    }

}
