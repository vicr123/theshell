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
#ifndef WIFISETTINGSPANE_H
#define WIFISETTINGSPANE_H

#include <QWidget>
#include "settingpane.h"

namespace Ui {
    class WifiSettingsPane;
}

struct WifiSettingsPanePrivate;
class WifiSettingsPane : public SettingPane
{
        Q_OBJECT

    public:
        explicit WifiSettingsPane(NetworkManager::Connection::Ptr connection, QWidget *parent = nullptr);
        ~WifiSettingsPane();

    private slots:
        void updateFields();

        void on_ssidField_textEdited(const QString &arg1);

        void on_modeComboBox_currentIndexChanged(int index);

        void on_mtuBox_valueChanged(int arg1);

        void on_hiddenNetworkSwitch_toggled(bool checked);

        void on_securityComboBox_currentIndexChanged(int index);

        void on_EnterpriseAuthMethod_currentIndexChanged(int index);

    private:
        Ui::WifiSettingsPane *ui;
        WifiSettingsPanePrivate* d;
};

#endif // WIFISETTINGSPANE_H
