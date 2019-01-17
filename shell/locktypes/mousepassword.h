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

#ifndef MOUSEPASSWORD_H
#define MOUSEPASSWORD_H

#include <QWidget>
#include <QByteArray>
#include <QMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QDir>
#include <QFile>
#include <unistd.h>
#include <polkit-qt5-1/PolkitQt1/Authority>
#include <QMessageBox>
#include <QProcess>
#include <ttoast.h>

namespace Ui {
    class MousePassword;
}

class MousePassword : public QWidget
{
        Q_OBJECT

    public:
        explicit MousePassword(QWidget *parent = 0);
        ~MousePassword();

    private slots:
        void on_resetMousePasswordButton_clicked();

        void on_backButton_clicked();

        void setMousePasswordLabel();

        void on_nextButton_clicked();

    signals:
        void exit();

    private:
        Ui::MousePassword *ui;

        void mousePressEvent(QMouseEvent* event);
        void mouseDoubleClickEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);

        QByteArray currentMousePassword, tentativeMousePassword;
        bool isListening = true;
        int stage = 0;

        const char* instructions[4] = {
            QT_TR_NOOP("To get started, use the mouse to input a sequence of button events now."),
            QT_TR_NOOP("Now, confirm the Mouse Password you chose by entering it again."),
            QT_TR_NOOP("Your Mouse Password is ready to be saved."),
            "done"
        };
};

#endif // MOUSEPASSWORD_H
