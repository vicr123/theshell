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
#ifndef CONNECTIONEDITOR_H
#define CONNECTIONEDITOR_H

#include <QWidget>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Connection>

namespace Ui {
    class ConnectionEditor;
}

struct ConnectionEditorPrivate;
class ConnectionEditor : public QWidget
{
        Q_OBJECT

    public:
        explicit ConnectionEditor(NetworkManager::Device::Ptr device, NetworkManager::Connection::Ptr connection, QWidget *parent = nullptr);
        ~ConnectionEditor();

    private slots:
        void on_connectionNameLineEdit_textChanged(const QString &arg1);

        void on_settingsList_currentRowChanged(int currentRow);

        void on_saveSettingsButton_clicked();

        void on_discardSettingsButton_clicked();

        void updateSettings();

        void on_connectAutomaticallySwitch_toggled(bool checked);

        void on_removeButton_clicked();

    private:
        Ui::ConnectionEditor *ui;
        ConnectionEditorPrivate* d;

        NMVariantMapMap currentSettings();
        void updateNmSettings();
};

#endif // CONNECTIONEDITOR_H
