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
#ifndef DEVICEPANEL_H
#define DEVICEPANEL_H

#include <QWidget>
#include <QDBusObjectPath>

namespace Ui {
    class DevicePanel;
}

struct DevicePanelPrivate;
class DevicePanel : public QWidget
{
    Q_OBJECT

    public:
        explicit DevicePanel(QDBusObjectPath device, QWidget* popoverOnWidget, QWidget* parent = 0);
        ~DevicePanel();

        enum DevicePanelType {
            Unknown,
            Wired,
            Wifi,
            Bluetooth,
            Cellular
        };

        DevicePanelType deviceType();

        QIcon chunkIcon();
        QString chunkText();

    public slots:
        void updateInfo();

    signals:
        void connectToWirelessDevice(QDBusObjectPath device);
        void getInformationAboutDevice(QDBusObjectPath device);
        void updated();

    private slots:
        void on_infoButton_clicked();

    private:
        DevicePanelPrivate* d;
        Ui::DevicePanel *ui;
};

#endif // DEVICEPANEL_H
