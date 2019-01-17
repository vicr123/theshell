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

#ifndef BTHANDSFREE_H
#define BTHANDSFREE_H

#include <QWidget>
#include <QBoxLayout>
#include <QLine>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QApplication>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

class BTHandsfree : public QWidget
{
    Q_OBJECT
public:
    explicit BTHandsfree(QWidget *parent = 0);

signals:

public slots:
    void detectDevices();
    QList<QString> getDevices();
    void placeCall(int deviceIndex, QString number);

private:
    QBoxLayout* layout;
    QLabel* infoLabel;
    QPushButton* hangupButton;
    QList<QDBusInterface*> interfaces;
    QDBusInterface* hangUpButtonInterface;
    QStringList knownDevices;
};

#endif // BTHANDSFREE_H
