#ifndef STATUSCENTERPANEOBJECT_H
#define STATUSCENTERPANEOBJECT_H

#include <QObject>
#include <QWidget>
#include <QIcon>
#include <functional>

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

        InformationalStatusCenterPaneObjectAttributes informationalAttributes;
        SettingStatusCenterPaneObjectAttributes settingAttributes;

        std::function<void(QString, QVariantList)> sendMessage;
        bool showing = false;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StatusCenterPaneObject::StatusPaneTypes)

#endif // STATUSCENTERPANEOBJECT_H
