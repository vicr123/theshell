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

#ifndef STATUSCENTERPANEOBJECT_H
#define STATUSCENTERPANEOBJECT_H

#include <QObject>
#include <QWidget>
#include <QIcon>
#include <functional>
#include <QVariant>
#include <QMetaType>

struct InformationalStatusCenterPaneObjectAttributes {
    QColor lightColor;
    QColor darkColor;
};

struct SettingStatusCenterPaneObjectAttributes {
    QIcon icon;
    bool providesLeftPane;
    QWidget* menuWidget = nullptr;
};

class StatusCenterPaneObject
{
    public:
        enum StatusPaneType {
            None = 0x0,
            Informational = 0x1,
            Setting = 0x2
        };
        Q_DECLARE_FLAGS(StatusPaneTypes, StatusPaneType)

        ~StatusCenterPaneObject() {}

        virtual QWidget* mainWidget() = 0;
        virtual QString name() = 0;
        virtual StatusPaneTypes type() = 0;
        virtual int position() = 0;
        virtual void message(QString name, QVariantList args = QVariantList()) = 0;
        virtual QVariant messageReturn(QString name, QVariantList args = QVariantList()) {
            return QVariant();
        }

        InformationalStatusCenterPaneObjectAttributes informationalAttributes;
        SettingStatusCenterPaneObjectAttributes settingAttributes;

        std::function<void(QString, QVariantList)> sendMessage;
        std::function<QVariant(QString)> getProperty;
        bool showing = false;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(StatusCenterPaneObject::StatusPaneTypes)
Q_DECLARE_METATYPE(StatusCenterPaneObject*)

#endif // STATUSCENTERPANEOBJECT_H
