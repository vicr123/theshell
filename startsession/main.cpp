#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QDebug>

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
        if (!QFile(desktopFile.value("TryExec").toString()).exists()) {
            return;
        }
    }

    if (desktopFile.value("hidden", false).toBool()) {
        return;
    }

    //Execute the Exec parameter
    qDebug() << "Starting " + desktopFile.value("Exec").toString();
    QProcess::startDetached(desktopFile.value("Exec").toString());
}

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

    //Set DPI
    QProcess::execute("xrandr --dpi " + QString::number(settings.value("screen/dpi", 96).toInt()));

    QProcess* tsProcess = new QProcess();
    #ifdef BLUEPRINT
        tsProcess->start("theshellb");
    #else
        tsProcess->start("theshell");
    #endif

    QObject::connect(tsProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus exitStatus){
        if (exitCode == 0) {
            QCoreApplication::exit(0);
        } else {
            //Restart theShell
            if (QMessageBox::question(NULL, "theShell crashed!", "theShell has crashed. Do you want to restart theShell?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
                #ifdef BLUEPRINT
                    tsProcess->start("theshellb");
                #else
                    tsProcess->start("theshell");
                #endif
            } else {
                QCoreApplication::exit(0);
            }
        }
    });

    QSettings::Format desktopFileFormat = QSettings::registerFormat("desktop", [](QIODevice &device, QSettings::SettingsMap &map) -> bool {
        QString group;
        while (!device.atEnd()) {
            QString line = device.readLine().trimmed();
            if (line.startsWith("[") && line.endsWith("]")) {
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

    return a.exec();
}
