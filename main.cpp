#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

extern "C" {
#include <libavdevice/avdevice.h>
}
#include <SDL2/SDL.h>

int main(int argc, char *argv[])
{
    avdevice_register_all();
    qDebug() << "avdevice_register_all";
    SDL_version v;
    SDL_VERSION(&v);

    qDebug()<< "SDL2_version" << v.major << v.minor << v.patch;
    qDebug() << "avdevice_version =" << avdevice_version();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
