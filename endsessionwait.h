#ifndef ENDSESSIONWAIT_H
#define ENDSESSIONWAIT_H

#include <QDialog>
#include <QProcess>
#include <QMediaPlayer>
#include <QSettings>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QFile>
#include <QDir>
#include <QDBusMessage>
#include <QDBusConnection>
#include "window.h"

namespace Ui {
class EndSessionWait;
}

class EndSessionWait : public QDialog
{
    Q_OBJECT

public:
    enum shutdownType {
        powerOff,
        reboot,
        logout,
        ask,
        dummy //FOR TESTING
    };

    explicit EndSessionWait(shutdownType type, QWidget *parent = 0);
    ~EndSessionWait();

    void showFullScreen();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void EndSessionNow();
    void on_CancelAsk_clicked();

    void on_PowerOff_clicked();

    void on_Reboot_clicked();

    void on_LogOut_clicked();

public slots:
    void close();

private:
    Ui::EndSessionWait *ui;

    void performEndSession();
    shutdownType type;
};

#endif // ENDSESSIONWAIT_H
