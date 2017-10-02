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
#include "nativeeventfilter.h"
#include <ttoast.h>

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

public slots:
    void updateGlobals();

signals:
    void updateBarDisplay(QString text, QIcon icon);

private:
    Ui::NetworkWidget *ui;

    QDBusInterface* nmInterface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
};

#endif // NETWORKWIDGET_H
