#ifndef STATUSCENTERPANEOBJECT_H
#define STATUSCENTERPANEOBJECT_H

#include <QObject>
#include <QWidget>
#include <QIcon>

struct InformationalStatusCenterPaneObjectAttributes {
    QColor lightColor;
    QColor darkColor;
};

struct SettingStatusCenterPaneObjectAttributes {
    QIcon icon;
};

class StatusCenterPaneObject
{
    public:
        enum StatusPaneType {
            Informational,
            Setting
        };
        Q_DECLARE_FLAGS(StatusPaneTypes, StatusPaneType)

        ~StatusCenterPaneObject() {}

        virtual QWidget* mainWidget() = 0;
        virtual QString name() = 0;
        virtual StatusPaneTypes type() = 0;
        virtual int position() = 0;
        InformationalStatusCenterPaneObjectAttributes informationalAttributes;
        SettingStatusCenterPaneObjectAttributes settingAttributes;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StatusCenterPaneObject::StatusPaneTypes)

#endif // STATUSCENTERPANEOBJECT_H
