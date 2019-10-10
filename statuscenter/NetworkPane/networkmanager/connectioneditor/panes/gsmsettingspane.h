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
#ifndef GSMSETTINGSPANE_H
#define GSMSETTINGSPANE_H

#include <QWidget>
#include "settingpane.h"

namespace Ui {
    class GsmSettingsPane;
}

struct GsmSettingsPanePrivate;
class GsmSettingsPane : public SettingPane
{
        Q_OBJECT

    public:
        explicit GsmSettingsPane(NetworkManager::Connection::Ptr connection, QWidget *parent = nullptr);
        ~GsmSettingsPane();

    public slots:
        void updateFields();

    private slots:
        void on_apnField_textChanged(const QString &arg1);

        void on_usernameField_textChanged(const QString &arg1);

        void on_passwordField_textChanged(const QString &arg1);

        void on_allowRoamingSwitch_toggled(bool checked);

        void on_numberField_textChanged(const QString &arg1);

    private:
        Ui::GsmSettingsPane *ui;

        GsmSettingsPanePrivate* d;
};

#endif // GSMSETTINGSPANE_H
