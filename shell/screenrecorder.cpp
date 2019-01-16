/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

#include "screenrecorder.h"
#include <QDebug>

#include "notificationsdbusadaptor.h"

ScreenRecorder::ScreenRecorder(QObject *parent) : QObject(parent)
{
    recorderProcess = new QProcess();
    //recorderProcess->setProcessChannelMode(QProcess::ForwardedChannels);
    connect(recorderProcess, SIGNAL(finished(int)), this, SLOT(recorderFinished(int)));
}

void ScreenRecorder::start() {
    if (!recording()) {
        if (!QFile("/usr/bin/ffmpeg").exists()) {
            NotificationsDBusAdaptor::Notify("theShell", 0, "", tr("Screen Recorder"), tr("To record your screen, you'll need to install ffmpeg"), QStringList(), QVariantMap(), -1);
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
            QString command = "ffmpeg -y -video_size %1 -f x11grab -i %2.0+%3,%4 %5/.screenRecording.mp4";
            command = command.arg(QString::number(currentScreen->geometry().width()) + "x" + QString::number(currentScreen->geometry().height()));
            command = command.arg(QString(qgetenv("DISPLAY")));
            command = command.arg(QString::number(currentScreen->geometry().left()));
            command = command.arg(QString::number(currentScreen->geometry().top()));
            command = command.arg(QDir::homePath());
            //command = command.arg("~");

            recorderProcess->start(command);
            s = Recording;
            emit stateChanged(Recording);
        } else {
            NotificationsDBusAdaptor::Notify("theShell", 0, "", tr("Screen Recorder"), tr("Couldn't start screen recording"), QStringList(), QVariantMap(), -1);
        }

    }
}

void ScreenRecorder::stop() {
    if (s == Recording) {
        recorderProcess->terminate();

        s = Processing;
        emit stateChanged(Processing);
    }
}

bool ScreenRecorder::recording() {
    return s == Recording;
}

void ScreenRecorder::recorderFinished(int returnCode) {
    s = Idle;
    emit stateChanged(Idle);

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
        NotificationsDBusAdaptor::Notify("theShell", 0, "", tr("Screen Recorder"), tr("Screen Recording saved in Recordings folder"), actions, QVariantMap(), -1)->then([=](uint nId) {
            connect(NotificationsDBusAdaptor::instance(), &NotificationsDBusAdaptor::ActionInvoked, [=](uint id, QString key) {
                if (id == nId) {
                    if (key == "view") {
                        QProcess::startDetached("xdg-open \"" + filename + "\"");
                    } else if (key == "del") {
                        QFile(filename).remove();
                    }
                }
            });
        });
    } else {
        NotificationsDBusAdaptor::Notify("theShell", 0, "", tr("Screen Recorder"), tr("Screen Recording failed"), QStringList(), QVariantMap(), -1);
    }
}
