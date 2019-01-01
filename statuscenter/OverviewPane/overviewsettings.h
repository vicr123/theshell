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

#ifndef OVERVIEWSETTINGS_H
#define OVERVIEWSETTINGS_H

#include <QWidget>
#include <statuscenterpaneobject.h>
#include <QSettings>

namespace Ui {
    class OverviewSettings;
}

class OverviewSettings : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit OverviewSettings(QWidget *parent = nullptr);
        ~OverviewSettings();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void on_weatherCheckBox_toggled(bool checked);

        void on_celsiusRadio_toggled(bool checked);

        void on_fahrenheitRadio_toggled(bool checked);

    private:
        Ui::OverviewSettings *ui;

        QSettings settings;
};

#endif // OVERVIEWSETTINGS_H
