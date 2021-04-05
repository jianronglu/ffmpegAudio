#ifndef PLAYAUDIOTHREAD_H
#define PLAYAUDIOTHREAD_H

#include <QThread>

class PlayAudioThread : public QThread
{
    Q_OBJECT
public:
    explicit PlayAudioThread(QObject *parent = nullptr);
    ~PlayAudioThread();

private:
    void run();

signals:

};

#endif // PLAYAUDIOTHREAD_H
