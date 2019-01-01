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

#ifndef BLUETOOTHMANAGEMENT_H
#define BLUETOOTHMANAGEMENT_H

#include <QStackedWidget>
#include <BluezQt/Adapter>
#include <BluezQt/PendingCall>
#include <BluezQt/Services>
#include <BluezQt/ObexObjectPush>
#include <BluezQt/ObexSession>
#include <ttoast.h>
#include <QLabel>
#include <QCheckBox>
#include <QInputDialog>
#include <QMessageBox>
#include <statuscenterpaneobject.h>
#include <QCommandLinkButton>
#include <QFileDialog>
#include <QProgressBar>
#include "btagent.h"
#include "btobexagent.h"
#include "deviceslistmodel.h"
#include "transferslistmodel.h"

namespace Ui {
    class BluetoothManagement;
}

class BluetoothManagement : public QStackedWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit BluetoothManagement(QWidget *parent = 0);
        ~BluetoothManagement();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void updateAdapters();

        void on_visibilityCheck_toggled(bool checked);

        void on_enableCheck_toggled(bool checked);

        void on_changeNameButton_clicked();

        void selectedDeviceChanged(QModelIndex current, QModelIndex previous);

        void on_mainMenuButton_clicked();

        void on_backButton_clicked();

        void on_pairButton_clicked();

        void on_unpairDeviceButton_clicked();

        void on_pairDeviceList_activated(const QModelIndex &index);

        void on_cancelPairingButton_clicked();

        void pairRequest(QString pin, BluezQt::Request<> req, QString explanation, bool showContinueButton);

        void pairReturnRequest(BluezQt::Request<QString> req, QString explanation);

        void pairCancel();

        void on_acceptCodeInputButton_clicked();

        void on_acceptCodeButton_clicked();

        void on_deviceNameEdit_textChanged(const QString &arg1);

        void loadCurrentDevice();

        void on_connectButton_clicked();

        void on_connectPANButton_clicked();

        void on_backButton_2_clicked();

        void on_activeFileTransfersButton_clicked();

        void on_sendFilesButton_clicked();

        void selectedTransferChanged(QModelIndex current, QModelIndex previous);

        void loadCurrentTransfer();

        void on_cancelTransferButton_clicked();

    private:
        Ui::BluetoothManagement *ui;

        BTAgent* agent;
        BTObexAgent* obexAgent;
        BluezQt::Manager* mgr;
        BluezQt::AdapterPtr currentAdapter;
        BluezQt::DevicePtr currentDevice, pairingDevice;
        BluezQt::Request<> currentRequest;
        BluezQt::Request<QString> currentReturnRequest;

        BluezQt::ObexManager* obexMgr;
        BluezQt::ObexTransferPtr currentTransfer;
        TransfersListModel* obexTransferModel;
        QMetaObject::Connection transferTransferredSignal, transferStateChangedSignal;

        void changeEvent(QEvent* event);

        void connectAllProfiles(int profileIndex, BluezQt::DevicePtr device);
};

#endif // BLUETOOTHMANAGEMENT_H
