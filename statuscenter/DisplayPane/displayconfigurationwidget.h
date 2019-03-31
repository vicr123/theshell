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
#ifndef DISPLAYCONFIGURATIONWIDGET_H
#define DISPLAYCONFIGURATIONWIDGET_H

#include <QFrame>
#include <X11/extensions/Xrandr.h>

namespace Ui {
    class DisplayConfigurationWidget;
}

struct DisplayConfigurationWidgetPrivate;
class DisplayConfigurationWidget : public QFrame
{
        Q_OBJECT

    public:
        explicit DisplayConfigurationWidget(QWidget *parent = nullptr);
        ~DisplayConfigurationWidget();

        void setPowered(bool powered);
        void setModes(QList<XRRModeInfo> modes);
        void setCurrentMode(XRRModeInfo mode);
        void setIsDefault(bool isDefault);
        void setDisplayName(QString displayName);

        RRMode mode();
        bool powered();

    private slots:
        void on_resBox_currentIndexChanged(int index);

        void on_displayPower_toggled(bool checked);

        void on_defaultButton_toggled(bool checked);

    signals:
        void resolutionChanged(QSize resolution);
        void poweredChanged(bool powered);
        void setDefault();

    private:
        Ui::DisplayConfigurationWidget *ui;

        DisplayConfigurationWidgetPrivate* d;
        void changeEvent(QEvent* event);
};

#undef Bool

#endif // DISPLAYCONFIGURATIONWIDGET_H
