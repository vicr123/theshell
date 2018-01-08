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

#ifndef MENU_H
#define MENU_H

#include <QDialog>
#include <QFocusEvent>
#include <QTimer>
#include <tpropertyanimation.h>
#include <QList>
#include <QListWidgetItem>
#include <QPaintEvent>
#include <QDrag>
#include <QCommandLinkButton>
#include <QStyledItemDelegate>
#include <systemd/sd-login.h>
#include "endsessionwait.h"
#include "mainwindow.h"
#include "dbusevents.h"
#include "tutorialwindow.h"
#include "bthandsfree.h"
#include <QFutureWatcher>
#include "apps/appslistmodel.h"

#undef KeyPress

namespace Ui {
class Menu;
}

class Menu : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit Menu(BTHandsfree* bt, QWidget *parent = 0);
    ~Menu();
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

    void show();
    void close();

signals:
    void menuClosing();

    void currentSettingChanged(bool isOn);

private slots:
    void checkForclose();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_commandLinkButton_clicked();

    void on_commandLinkButton_2_clicked();

    void on_commandLinkButton_3_clicked();

    void on_lineEdit_textEdited(const QString &arg1);

    bool eventFilter(QObject *object, QEvent *event);

    void on_lineEdit_returnPressed();

    void on_pushButton_3_clicked();

    void on_commandLinkButton_5_clicked();

    void on_commandLinkButton_7_clicked();

    void on_commandLinkButton_8_clicked();

    void on_commandLinkButton_6_clicked();

    void on_commandLinkButton_4_clicked();

    void on_exitButton_clicked();

    void on_fakeEndButton_clicked();

    void on_helpButton_clicked();

    void on_reportBugButton_clicked();

    void launchAppByIndex(const QModelIndex &index);

    void showActionMenuByIndex(QModelIndex index);

    void on_appsListView_customContextMenuRequested(const QPoint &pos);

private:
    Ui::Menu *ui;

    bool checkFocus(QLayout *layout);
    QSettings settings;

    //QList<App> apps;
    //QList<App> appsShown;
    int pinnedAppsCount = 0;

    bool doCheckForClose = false;

    //void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent* event);
    void changeEvent(QEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    void reject();
    bool resizing = false;

    BTHandsfree* bt;
};

#endif // MENU_H
