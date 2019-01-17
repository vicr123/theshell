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

#ifndef AUTHENTICATE_H
#define AUTHENTICATE_H

#include <QDialog>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QIcon>
#include <QTimer>
#include <QDateTime>
#include <QDBusInterface>
#include <QDesktopWidget>
#include <QApplication>
#include <QProcess>
#include <QPushButton>
#include <QComboBox>
#include "tvirtualkeyboard.h"
#include <QX11Info>
#include <polkit-qt5-1/PolkitQt1/Identity>
#include <polkit-qt5-1/PolkitQt1/Details>
#include <QDebug>
#include <QStackedWidget>
#include <QListWidget>
#include <ttoast.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#undef None
#undef Bool

namespace Ui {
class Authenticate;
}

class Authenticate : public QDialog
{
    Q_OBJECT

public:
    explicit Authenticate(QWidget *parent = 0);
    ~Authenticate();

    QString getPassword();
    void showFullScreen(bool showError = false);

    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

signals:
    void okClicked();

    void newUser(PolkitQt1::Identity newUser);

public slots:
    void setMessage(QString message);

    void setUser(QString user);

    void setUser(PolkitQt1::Identity user);

    void setUsers(PolkitQt1::Identity::List users);

    void setIcon(QIcon icon);

    void setDetails(const PolkitQt1::Details& details);

    void reject();

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_lineEdit_returnPressed();

    void on_keyboardButton_clicked();

    void on_backToMain_clicked();

    void on_switchUser_clicked();

    void on_authenticationUsers_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    private:
    Ui::Authenticate *ui;
    QGraphicsOpacityEffect *fadeEffect;
};
Q_DECLARE_METATYPE(PolkitQt1::Identity)

#endif // AUTHENTICATE_H
