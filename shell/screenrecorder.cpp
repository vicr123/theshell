#include "screenrecorder.h"
#include <QDebug>

extern NotificationsDBusAdaptor* ndbus;

ScreenRecorder::ScreenRecorder(QObject *parent) : QObject(parent)
{
    recorderProcess = new QProcess();
    //recorderProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(recorderProcess, SIGNAL(finished(int)), this, SLOT(recorderFinished(int)));
}

void ScreenRecorder::start() {
    if (!recording()) {
        if (!QFile("/usr/bin/ffmpeg").exists()) {
            ndbus->Notify("theShell", 0, "", tr("Screen Recorder"), tr("To record your screen, you'll need to install ffmpeg"), QStringList(), QVariantMap(), -1);
            return;
        }

        //Build command
        QScreen* currentScreen = NULL;
        for (QScreen* screen : QApplication::screens()) {
            if (screen->geometry().contains(QCursor::pos())) {
                currentScreen = screen;
            }
        }

        if (currentScreen != NULL) {
            QString command = "ffmpeg -y -video_size %1 -f x11grab -i %2.0+0,0 %3/.screenRecording.mp4";
            command = command.arg(QString::number(currentScreen->geometry().width()) + "x" + QString::number(currentScreen->geometry().height()));
            command = command.arg(QString(qgetenv("DISPLAY")));
            command = command.arg(QDir::homePath());
            //command = command.arg("~");

            recorderProcess->start(command);
            isRecording = true;
            emit recordingChanged(true);
        } else {
            ndbus->Notify("theShell", 0, "", tr("Screen Recorder"), tr("Couldn't start screen recording"), QStringList(), QVariantMap(), -1);
        }

    }
}

void ScreenRecorder::stop() {
    if (isRecording) {
        recorderProcess->terminate();
    }
}

bool ScreenRecorder::recording() {
    return this->isRecording;
}

void ScreenRecorder::recorderFinished(int returnCode) {
    isRecording = false;
    emit recordingChanged(false);

    qDebug() << returnCode;
    if (returnCode == 0 || returnCode == 255) {
        QDir::home().mkdir("Recordings");
        QFile f(QDir::homePath() + "/.screenRecording.mp4");
        f.rename(QDir::homePath() + "/Recordings/" + QDateTime::currentDateTime().toString("hh-mm-ss-yyyy-MM-dd") + ".mp4");

        QString filename = f.fileName();

        QStringList actions;
        actions.append("view");
        actions.append("View");
        actions.append("del");
        actions.append("Delete");
        uint nId = ndbus->Notify("theShell", 0, "", tr("Screen Recorder"), tr("Screen Recording saved in Recordings folder"), actions, QVariantMap(), -1);
        connect(ndbus, &NotificationsDBusAdaptor::ActionInvoked, [=](uint id, QString key) {
            if (id == nId) {
                if (key == "view") {
                    QProcess::startDetached("xdg-open \"" + filename + "\"");
                } else if (key == "del") {
                    QFile(filename).remove();
                }
            }
        });
    } else {
        ndbus->Notify("theShell", 0, "", tr("Screen Recorder"), tr("Screen Recording failed"), QStringList(), QVariantMap(), -1);
    }
}
