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
#include <QDrag>
#include <QCommandLinkButton>
#include <systemd/sd-login.h>
#include "endsessionwait.h"
#include "app.h"
#include "mainwindow.h"
#include "thewaveworker.h"
#include "dbusevents.h"
#include "tutorialwindow.h"

#undef KeyPress

class theWaveWorker;

class theWaveFrame : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString backupText READ backupText WRITE setBackupText)
public:
    explicit theWaveFrame(QWidget *parent = 0);

signals:

public slots:
    QString backupText();
    void setBackupText(QString text);
private:
    QPoint startPosition;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    QString bText;
};

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

    void show(bool openTotheWave = false, bool startListening = false);
    void close();

    QFrame* theWaveFrame;

signals:
    void menuClosing();

    void thewave_processText(QString text, bool isText = false);

    void thewave_sayLaunchApp(QString app);

    void thewave_sayLaunchApp_disambiguation(QStringList apps);

    void thewaveBegin();

    void thewaveStop();

    void currentSettingChanged(bool isOn);

public slots:
    void collapse();

    void uncollapse();

private slots:
    void checkForclose();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_commandLinkButton_clicked();

    void on_commandLinkButton_2_clicked();

    void on_commandLinkButton_3_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

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

    void thewaveLoudnessChanged(qreal loudness);

    void on_commandLinkButton_6_clicked();

    void on_commandLinkButton_4_clicked();

    void on_listentheWave_clicked();

    void on_thewave_launch_disambiguation_itemClicked(QListWidgetItem *item);

    void showSettingFrame(QIcon icon, QString text, bool isOn);

    void on_thewaveSettingsFrame_Switch_toggled(bool checked);

    void showMathematicsFrame(QString expression, QString answer);

    void showMediaFrame(QPixmap art, QString title, QString artist, bool isPlaying);

    void on_thewaveMedia_Next_clicked();

    void on_thewaveMedia_Play_clicked();

    void on_thewaveMedia_Back_clicked();

    void on_listWidget_customContextMenuRequested(const QPoint &pos);

    void on_exitButton_clicked();

    void on_fakeEndButton_clicked();

private:
    Ui::Menu *ui;

    bool checkFocus(QLayout *layout);
    QSettings settings;

    QList<App> apps;
    QList<App> appsShown;
    int pinnedAppsCount = 0;

    bool doCheckForClose = false;
    bool istheWaveOpen = false;

    //void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent* event);
    void changeEvent(QEvent* event);

    void reject();

    theWaveWorker* waveWorker;
    bool isListening = false;
    bool istheWaveReady = false;
};

#endif // MENU_H
