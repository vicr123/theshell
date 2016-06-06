#ifndef MENU_H
#define MENU_H

#include <QDialog>
#include <QFocusEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QDir>
#include <QList>
#include <QListWidgetItem>
#include <QDirIterator>
#include <QPaintEvent>
#include <QPainter>
#include <QMimeType>
#include <QMimeDatabase>
#include <systemd/sd-login.h>
#include "endsessionwait.h"
#include "app.h"
#include "mainwindow.h"
#include "thewaveworker.h"

#undef KeyPress

namespace Ui {
class Menu;
}

class Menu : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit Menu(QWidget *parent = 0);
    ~Menu();
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

    void show(bool openTotheWave = false);
    void close();

signals:
    void appOpening(QString name, QIcon icon);
    void menuClosing();

    void thewave_processText(QString text, bool isText = false);

private slots:
    void checkForclose();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_commandLinkButton_clicked();

    void on_commandLinkButton_2_clicked();

    void on_commandLinkButton_3_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_textEdited(const QString &arg1);

    bool eventFilter(QObject *object, QEvent *event);

    void on_lineEdit_returnPressed();

    void on_pushButton_3_clicked();

    void on_commandLinkButton_5_clicked();

    void on_commandLinkButton_7_clicked();

    void on_commandLinkButton_8_clicked();

    void on_activateTheWave_clicked();

    void on_closetheWaveButton_clicked();

    void showCallFrame(bool emergency);

    void resetFrames();

    void showMessageFrame();

    void showHelpFrame();

    void showWikipediaFrame(QString title, QString text);

    void thewave_launchapp(QString app);

    void on_thewave_line_returnPressed();

    void on_closetheWaveButton_2_clicked();

    void on_thewave_launch_launchapp_clicked();

    void thewave_outputSpeech(QString speech);

    void showFlightFrame(QString flight);

private:
    Ui::Menu *ui;

    bool checkFocus(QLayout *layout);
    QSettings settings;

    QList<App*> *apps;
    QList<App*> *appsShown;

    bool doCheckForClose = false;

    //void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent* event);
    void changeEvent(QEvent* event);

    theWaveWorker* waveWorker;
    bool isListening;
    bool istheWaveReady = false;
};

#endif // MENU_H
