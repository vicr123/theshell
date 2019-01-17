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

#ifndef NETWORKWIDGET_H
#define NETWORKWIDGET_H

#include <QWidget>
#include <QDBusInterface>
#include <QFrame>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QDBusArgument>
#include <QTableWidget>
#include <QTreeWidget>
#include <functional>
#include <QDebug>
#include "availablenetworkslist.h"
#include "savednetworkslist.h"
#include "nativeeventfilter.h"
#include <ttoast.h>
#include "infopanedropdown.h"

namespace Ui {
class NetworkWidget;
}

class DevicePanel : public QWidget
{
    Q_OBJECT

public:
    explicit DevicePanel(QDBusObjectPath device, QWidget* parent = 0);
    ~DevicePanel();

public slots:
    void updateInfo();

signals:
    void connectToWirelessDevice(QDBusObjectPath device);
    void getInformationAboutDevice(QDBusObjectPath device);

private:
    QDBusInterface* deviceInterface;
    QDBusInterface* nmInterface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    QLabel *iconLabel, *connectionNameLabel, *connectionSubNameLabel;
    QDBusObjectPath device;
    QBoxLayout* buttonLayout;
};

class NetworkWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit NetworkWidget(QWidget *parent = 0);
        ~NetworkWidget();

    public slots:
        void flightModeChanged(bool flight);

    private slots:
        void updateDevices();

        void on_networksBackButton_clicked();

        void on_SecurityBackButton_clicked();

        void on_AvailableNetworksList_clicked(const QModelIndex &index);

        void connectToWirelessDevice(QDBusObjectPath device);

        void getInformationAboutDevice(QDBusObjectPath device);

        void on_SecurityConnectButton_clicked();

        void on_networksManualButton_clicked();

        void on_SecurityType_currentIndexChanged(int index);

        void on_EnterpriseAuthMethod_currentIndexChanged(int index);

        QString selectCertificate();

        void on_EnterpriseTLSUserCertificateSelect_clicked();

        void on_EnterpriseTLSCACertificateSelect_clicked();

        void on_knownNetworksButton_clicked();

        void on_knownNetworksBackButton_clicked();

        void on_EnterprisePEAPCaCertificateSelect_clicked();

        void on_knownNetworksDeleteButton_clicked();

        void on_tetheringEnableTetheringButton_clicked();

        void on_tetheringBackButton_clicked();

        void on_tetheringButton_clicked();

    public slots:
        void updateGlobals();

    signals:
        void updateBarDisplay(QString text, QIcon icon);

    private:
        Ui::NetworkWidget *ui;

        void changeEvent(QEvent* event);

        QDBusInterface* nmInterface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
        bool flightMode = false;
};

#endif // NETWORKWIDGET_H
