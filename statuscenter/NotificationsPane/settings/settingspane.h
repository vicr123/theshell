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
#ifndef SETTINGSPANE_H
#define SETTINGSPANE_H

#include <QWidget>
#include <statuscenterpaneobject.h>

namespace Ui {
    class SettingsPane;
}

struct SettingsPanePrivate;
class SettingsPane : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit SettingsPane(QWidget *parent = nullptr);
        ~SettingsPane();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args = QVariantList());

    private slots:
        void on_backButton_clicked();

        void on_showContentsButton_toggled(bool checked);

        void on_hideContentsButton_toggled(bool checked);

        void on_dontShowButton_toggled(bool checked);

        void on_emphasiseSendingAppSwitch_toggled(bool checked);

        void on_notificationSoundBox_currentIndexChanged(int index);

        void on_attenuateOnNotification_toggled(bool checked);

        void on_allowNotificationsMasterSwitch_toggled(bool checked);

        void on_allowPopupsSwitch_toggled(bool checked);

        void on_allowSoundsSwitch_toggled(bool checked);

        void on_bypassQuietModeSwitch_toggled(bool checked);

        void on_connectMediaSwitch_toggled(bool checked);

        void on_chargingSwitch_toggled(bool checked);

        void on_unplugSwitch_toggled(bool checked);

        void on_notificationVolumeSlider_valueChanged(int value);

    private:
        Ui::SettingsPane *ui;
        SettingsPanePrivate* d;
};

#endif // SETTINGSPANE_H
