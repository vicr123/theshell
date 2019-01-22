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

#ifndef KDECONNECTWIDGET_H
#define KDECONNECTWIDGET_H

#include <QWidget>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QListView>
#include <QLabel>
#include <QStackedWidget>
#include <QCommandLinkButton>
#include <QDBusPendingCallWatcher>
#include <QMessageBox>
#include <ttoast.h>
#include <QFileDialog>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QDBusServiceWatcher>
#include <QProcess>
#include <QInputDialog>
#include "kdeconnectdevicesmodel.h"

#include <statuscenterpaneobject.h>

namespace Ui {
    class KdeConnectWidget;
}

class KdeConnectWidget : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit KdeConnectWidget(QWidget *parent = 0);
        ~KdeConnectWidget();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args = QVariantList());
    private slots:
        void kdeConnectAnnouncedNameChanged(QString name);

        void kdeConnectOnline();

        void kdeConnectGone();

        void selectedDeviceChanged(QModelIndex current, QModelIndex previous);
        void on_pingButton_clicked();

        void on_locateButton_clicked();

        void on_pairButton_clicked();

        void updateCurrentDevice();

        void on_sendFileButton_clicked();

        void on_settingsButton_clicked();

        void on_backButton_clicked();

        void on_pluginBatteryMonitor_toggled(bool checked);

        void on_pluginVirtualInput_toggled(bool checked);

        void on_pluginClipboard_toggled(bool checked);

        void on_pluginInhibitScreensaver_toggled(bool checked);

        void on_pluginMultimediaControlReceiver_toggled(bool checked);

        void on_pluginPauseMedia_toggled(bool checked);

        void on_pluginPing_toggled(bool checked);

        void on_pluginNotifications_toggled(bool checked);

        void on_pluginBrowseFilesystem_toggled(bool checked);

        void on_pluginRemoteKeyboard_toggled(bool checked);

        void on_pluginRingPhone_toggled(bool checked);

        void on_pluginRunCommands_toggled(bool checked);

        void on_pluginSendNotifications_toggled(bool checked);

        void on_pluginSendReceive_toggled(bool checked);

        void on_pluginTelephony_toggled(bool checked);

        void on_smsButton_clicked();

        void on_smsMessage_textChanged();

        void on_backButton_3_clicked();

        void on_sendSMSButton_clicked();

        void on_startkdeConnectButton_clicked();

        void on_encryptionButton_clicked();

        void on_smsNumber_textEdited(const QString &arg1);

        void on_renameButton_clicked();

    private:
        Ui::KdeConnectWidget *ui;

        QDBusInterface* daemon;
        QDBusServiceWatcher* watcher;
        QString currentId;
        KdeConnectDevicesModel* devicesModel;

        void changeEvent(QEvent* event);
};

#endif // KDECONNECTWIDGET_H
