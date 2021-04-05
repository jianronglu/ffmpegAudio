#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "audiothread.h"
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

void MainWindow::on_audioButton_clicked()
{
    if (!_audioThread) {
       _audioThread = new AudioThread(this);
       _audioThread->start();//会自动调用run

       connect(_audioThread, &AudioThread::finished, [this] () {//线程结束 (lanmbda 表达式 oc 中 block)
          _audioThread = nullptr;// 必须捕获 this 不然无法访问 _audioThread
          ui->audioButton->setText("开始录音");
       });
       qDebug() << "AudioThread Start";
       ui->audioButton->setText("结束录音");
    } else {
       _audioThread->requestInterruption();
       _audioThread = nullptr;
//       ui->audioButton->setText("开始录音");
    }
}

/*
➜  ~ ffmpeg -devices -hide_banner
Devices:
 D. = Demuxing supported
 .E = Muxing supported
 --
 D  avfoundation    AVFoundation input device
 D  lavfi           Libavfilter virtual input device
  E sdl,sdl2        SDL2 output device
 D  x11grab         X11 screen capture, using XCB
*/
/*
➜  ~ ffmpeg -f avfoundation -list_devices true  -i dummy -hide_banner
[AVFoundation indev @ 0x7fa29fe30000] AVFoundation video devices:
[AVFoundation indev @ 0x7fa29fe30000] [0] FaceTime HD Camera
[AVFoundation indev @ 0x7fa29fe30000] [1] Capture screen 0
[AVFoundation indev @ 0x7fa29fe30000] [2] Capture screen 1
[AVFoundation indev @ 0x7fa29fe30000] AVFoundation audio devices:
[AVFoundation indev @ 0x7fa29fe30000] [0] Built-in Microphone
*/
