#ifndef THEWAVE_H
#define THEWAVE_H

#include <QDialog>
#include <QDesktopWidget>
#include <QFrame>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <QPropertyAnimation>
#include <QMap>
#include "thewaveworker.h"
#include "infopanedropdown.h"

namespace Ui {
class theWave;
}

class theWave : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit theWave(InfoPaneDropdown* infoPane, QWidget *parent = 0);
    ~theWave();

    void show();

    void close();

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

private slots:
    void checkForClose();

    void on_pushButton_2_clicked();

    void resetFrames();

    void showCallFrame();
    void showMessageFrame();

    void on_pushButton_clicked();

signals:
    void startVoice();

private:
    Ui::theWave *ui;

    bool isListening;
    bool checkFocus(QLayout* layout);
    theWaveWorker *worker;

    InfoPaneDropdown *infoPane;
};

#endif // THEWAVE_H
