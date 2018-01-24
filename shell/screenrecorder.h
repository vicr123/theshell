#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <QObject>
#include <QProcess>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QX11Info>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include "notificationsWidget/notificationsdbusadaptor.h"

class ScreenRecorder : public QObject
{
        Q_OBJECT
    public:
        explicit ScreenRecorder(QObject *parent = nullptr);

        enum State {
            Idle,
            Recording,
            Processing
        };

    signals:
        void stateChanged(State state);


    public slots:
        void start();
        void stop();
        bool recording();

    private slots:
        void recorderFinished(int returnCode);

    private:
        QProcess* recorderProcess;
        State s = Idle;
};

#endif // SCREENRECORDER_H
