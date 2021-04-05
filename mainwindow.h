#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <audiothread.h>
#include <playaudiothread.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_recodeAudioBtn_clicked();

    void on_playAudioBtn_clicked();

private:
    Ui::MainWindow *ui;
    AudioThread *_audioThread = nullptr;
    PlayAudioThread *_playAudioThread = nullptr;
};
#endif // MAINWINDOW_H
