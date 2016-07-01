#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QDialog>
#include <QSettings>
#include <QMenu>
#include <QDesktopWidget>
#include <QSvgRenderer>
#include "mainwindow.h"
#include "choosebackground.h"

#include <X11/Xlib.h>

class ChooseBackground;

namespace Ui {
class Background;
}

class Background : public QDialog
{
    Q_OBJECT

public:
    explicit Background(MainWindow* mainwindow, QRect screenGeometry, QWidget *parent = 0);
    ~Background();

    void show();

private slots:
    void on_graphicsView_customContextMenuRequested(const QPoint &pos);

    void on_actionOpen_Status_Center_triggered();

    void on_actionOpen_theShell_Settings_triggered();

    void on_actionOpen_System_Settings_triggered();

    void on_actionChange_Background_triggered();

private:
    Ui::Background *ui;

    MainWindow* mainwindow;
};

#endif // BACKGROUND_H
