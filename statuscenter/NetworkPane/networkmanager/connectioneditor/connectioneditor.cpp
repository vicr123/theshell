/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#include "connectioneditor.h"
#include "ui_connectioneditor.h"

#include <QMessageBox>
#include <QDebug>
#include <ttoast.h>
#include <NetworkManagerQt/Settings>

#include "panes/gsmsettingspane.h"
#include "panes/wifisettingspane.h"

struct ConnectionEditorPrivate {
    NetworkManager::Device::Ptr device;
    NetworkManager::Connection::Ptr connection;

    NMVariantMapMap originalSettings;
};

ConnectionEditor::ConnectionEditor(NetworkManager::Device::Ptr device, NetworkManager::Connection::Ptr connection, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionEditor)
{
    ui->setupUi(this);
    d = new ConnectionEditorPrivate();

    ui->leftPane->setFixedWidth(SC_DPI(300));
    ui->unsavedSettingsWidget->setVisible(false);
    ui->discardSettingsButton->setProperty("type", "destructive");
    ui->removeButton->setProperty("type", "destructive");

    d->device = device;
    d->connection = connection;

    d->originalSettings = d->connection->settings()->toMap();

    for (NetworkManager::Setting::Ptr setting : d->connection->settings()->settings()) {
        SettingPane* pane = nullptr;
        switch (setting->type()) {
            case NetworkManager::Setting::Gsm:
                pane = new GsmSettingsPane(d->connection, this);
                break;
            case NetworkManager::Setting::Wireless:
                pane = new WifiSettingsPane(d->connection, this);
                break;
        }

        if (pane != nullptr) {
            connect(pane, &SettingPane::settingsChanged, this, [=] {
                updateNmSettings();
            });

            ui->settingsList->addItem(pane->windowTitle());
            ui->stackedWidget->addWidget(pane);
        }
    }

    connect(d->connection.data(), &NetworkManager::Connection::updated, this, [=] {
        ui->unsavedSettingsWidget->setVisible(d->connection->isUnsaved());
        updateSettings();
    });

    //Update the settings
    updateSettings();
    d->connection->update(d->originalSettings);
}

ConnectionEditor::~ConnectionEditor()
{
    delete ui;
    delete d;
}

void ConnectionEditor::on_connectionNameLineEdit_textChanged(const QString &arg1)
{
    d->connection->settings()->setId(arg1);
    updateNmSettings();
}

void ConnectionEditor::updateSettings() {
    ui->titleLabel->setText(d->connection->settings()->id());
    ui->connectionNameLineEdit->setText(d->connection->settings()->id());
    ui->connectAutomaticallySwitch->setChecked(d->connection->settings()->autoconnect());
}

void ConnectionEditor::on_settingsList_currentRowChanged(int currentRow)
{
    ui->stackedWidget->setCurrentIndex(currentRow);
}

void ConnectionEditor::on_saveSettingsButton_clicked()
{
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(d->connection->update(currentSettings()));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
        if (watcher->reply().type() == QDBusMessage::ErrorMessage) {
            tToast* toast = new tToast();
            toast->setTitle(tr("Couldn't save settings"));
            toast->setText(tr("NetworkManager replied: %1").arg(watcher->reply().errorName() + ": " + watcher->reply().errorMessage()));
            toast->show(this);
            connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
        }
        watcher->deleteLater();
    });
}

void ConnectionEditor::on_discardSettingsButton_clicked()
{
    d->connection->update(d->originalSettings);
}

void ConnectionEditor::on_connectAutomaticallySwitch_toggled(bool checked)
{
    d->connection->settings()->setAutoconnect(checked);
    updateNmSettings();
}

void ConnectionEditor::on_removeButton_clicked()
{
    if (QMessageBox::warning(this, tr("Remove Connection?"), tr("To use this connection again, you'll need to set it up again."), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        d->connection->remove();
    }
}

void ConnectionEditor::updateNmSettings() {
    d->connection->updateUnsaved(currentSettings());
}

NMVariantMapMap ConnectionEditor::currentSettings() {
    NMVariantMapMap map = d->connection->settings()->toMap();

    //NetworkManagerQt seems to not do this for some connections for whatever reason
    for (NetworkManager::Setting::Ptr setting : d->connection->settings()->settings()) {
        QVariantMap childMap = setting->toMap();
        if (!childMap.isEmpty()) {
            map.insert(setting->name(), childMap);
        }
    }

    return map;
}
