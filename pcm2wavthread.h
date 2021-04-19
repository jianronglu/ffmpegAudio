#ifndef PCM2WAVTHREAD_H
#define PCM2WAVTHREAD_H

#include <QThread>

class PCM2WAVThread : public QThread
{
    Q_OBJECT
public:
    explicit PCM2WAVThread(QObject *parent = nullptr);
    ~PCM2WAVThread();
signals:

private:
    void run();
};

#endif // PCM2WAVTHREAD_H
