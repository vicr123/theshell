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
#include <NetworkManagerQt/Settings>

#include "panes/gsmsettingspane.h"

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
        }

        if (pane != nullptr) {
            connect(pane, &SettingPane::settingsChanged, this, [=] {
                d->connection->updateUnsaved(d->connection->settings()->toMap());
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
    d->connection->updateUnsaved(d->connection->settings()->toMap());
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
    d->connection->save();
}

void ConnectionEditor::on_discardSettingsButton_clicked()
{
    d->connection->update(d->originalSettings);
}

void ConnectionEditor::on_connectAutomaticallySwitch_toggled(bool checked)
{
    d->connection->settings()->setAutoconnect(checked);
    d->connection->updateUnsaved(d->connection->settings()->toMap());
}

void ConnectionEditor::on_removeButton_clicked()
{
    if (QMessageBox::warning(this, tr("Remove Connection?"), tr("To use this connection again, you'll need to set it up again."), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        d->connection->remove();
    }
}
