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

    signals:
        void recordingChanged(bool recording);

    public slots:
        void start();
        void stop();
        bool recording();

    private slots:
        void recorderFinished(int returnCode);

    private:
        QProcess* recorderProcess;
        bool isRecording = false;
};

#endif // SCREENRECORDER_H
