/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QDBusConnection>
#include "errordialog.h"
#include "startmonitor.h"

void startupDesktopFile(QString path, QSettings::Format format) {
    QSettings desktopFile(path, format);
    desktopFile.beginGroup("Desktop Entry");

    if (desktopFile.contains("OnlyShowIn")) {
        QStringList desktops = desktopFile.value("OnlyShowIn").toString().split(";");
        if (!desktops.contains("theshell")) {
            return;
        }
    }

    if (desktopFile.contains("NotShowIn")) {
        QStringList desktops = desktopFile.value("NotShowIn").toString().split(";");
        if (desktops.contains("theshell")) {
            return;
        }
    }

    if (desktopFile.contains("TryExec")) {
        if (!QFile(desktopFile.value("TryExec").toString()).exists() && !QFile("/usr/bin/" + desktopFile.value("TryExec").toString()).exists()) {
            return;
        }
    }

    if (desktopFile.value("Hidden", false).toBool()) {
        return;
    }

    //Execute the Exec parameter
    qDebug() << "Starting " + desktopFile.value("Exec").toString();
    QProcess::startDetached(desktopFile.value("Exec").toString());
}

QProcess* tsProcess;
int errorCount = 0;
bool started = false;
QString theShellStdOut;

int main(int argc, char *argv[])
{
    //Put environment variables
    qputenv("XDG_CURRENT_DESKTOP", "KDE");
    qputenv("DE", "kde");
    qputenv("KDE_SESSION_VERSION", "5");
    qputenv("QT_QPA_PLATFORMTHEME", "ts");

    QApplication a(argc, argv);
    a.setOrganizationName("theSuite");
    a.setApplicationName("ts-startsession");
    a.setQuitOnLastWindowClosed(false);

    QSettings settings;
    QSettings tsSettings("theSuite", "theShell");

    //Set DPI
    QProcess::execute("xrandr --dpi " + QString::number(tsSettings.value("screen/dpi", 96).toInt()));

    StartMonitor* monitor = new StartMonitor;

    QDBusConnection::sessionBus().connect("org.thesuite.theshell", "/org/thesuite/theshell", "org.thesuite.theshell", "Ready", monitor, SLOT(MarkStarted()));
    QDBusConnection::sessionBus().connect("org.thesuite.theshell", "/org/thesuite/theshell", "org.thesuite.theshell", "ShowSplash", monitor, SLOT(ShowSplash()));
    QDBusConnection::sessionBus().connect("org.thesuite.theshell", "/org/thesuite/theshell", "org.thesuite.theshell", "HideSplash", monitor, SLOT(HideSplash()));

    monitor->ShowSplash();
    tsProcess = new QProcess();
    #ifdef BLUEPRINT
        tsProcess->start("theshellb --session-starter-running");
    #else
        tsProcess->start("theshell --session-starter-running");
    #endif

    QObject::connect(tsProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitCode == 0) {
            QCoreApplication::exit(0);
        } else {
            //Restart theShell

            monitor->HideSplash();
            errorCount++;
            ErrorDialog* d = new ErrorDialog(monitor->started(), errorCount, theShellStdOut);
            ErrorDialog::connect(d, &ErrorDialog::restart, [=] {
                d->deleteLater();

                monitor->MarkNotStarted();
                monitor->ShowSplash();

                #ifdef BLUEPRINT
                    tsProcess->start("theshellb --session-starter-running");
                #else
                    tsProcess->start("theshell --session-starter-running");
                #endif
            });
            ErrorDialog::connect(d, &ErrorDialog::logout, [=] {
                d->deleteLater();
                QCoreApplication::exit(0);
            });
            d->showFullScreen();
            theShellStdOut.clear();
        }
    });
    QObject::connect(tsProcess, &QProcess::readyReadStandardOutput, [=] {
        QString all = tsProcess->readAllStandardOutput();
        for (QString line : all.split("\n")) {
            if (!monitor->started()) {
                if (line.startsWith("QUESTION:")) {
                    QStringList split = line.split(":");
                    QString title = split.at(1);
                    QString message = split.at(2);
                    monitor->SplashQuestion(title, message);
                } else if (line.startsWith("PROMPT:")) {
                    QStringList split = line.split(":");
                    QString title = split.at(1);
                    QString message = split.at(2);
                    monitor->SplashPrompt(title, message.replace("[newln]", "\n"));
                }
            }
        }
        theShellStdOut.append(all);
    });
    QObject::connect(tsProcess, &QProcess::readyReadStandardError, [=] {
        QString out = tsProcess->readAllStandardError();
        theShellStdOut.append(out);
    });
    QObject::connect(monitor, &StartMonitor::questionResponse, [=](QString response) {
        tsProcess->write(response.toLocal8Bit());
    });

    if (!a.arguments().contains("--no-autostart")) {
        QSettings::Format desktopFileFormat = QSettings::registerFormat("desktop", [](QIODevice &device, QSettings::SettingsMap &map) -> bool {
            QString group;
            while (!device.atEnd()) {
                QString line = device.readLine().trimmed();
                if (line == "") {

                } else if (line.startsWith("[") && line.endsWith("]")) {
                    group = line.mid(1, line.length() - 2);
                } else {
                    QString key = line.left(line.indexOf("="));
                    QString value = line.mid(line.indexOf("=") + 1);
                    map.insert(group + "/" + key, value);
                }
            }
            return true;
        }, [](QIODevice &device, const QSettings::SettingsMap &map) -> bool {
            return false;
        }, Qt::CaseInsensitive);

        //Start system daemons
        QProcess* polkitProcess = new QProcess();

#ifdef BLUEPRINT
        polkitProcess->start("/usr/lib/ts-polkitagentb");
#else
        polkitProcess->start("/usr/lib/ts-polkitagent");
#endif

        //Start startup applications
        QStringList knownFileNames;
        QDir autostartDir(QDir::homePath() + "/.config/autostart");
        for (QString fileName : autostartDir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
            startupDesktopFile(QDir::homePath() + "/.config/autostart/" + fileName, desktopFileFormat);
            knownFileNames.append(fileName);
        }

        autostartDir.cd("/etc/xdg/autostart");
        for (QString fileName : autostartDir.entryList(QDir::NoDotAndDotDot | QDir::Files)) {
            if (!knownFileNames.contains(fileName)) {
                startupDesktopFile("/etc/xdg/autostart/" + fileName, desktopFileFormat);
            }
        }
    }

    return a.exec();
}
