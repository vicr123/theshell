#ifndef AUTOROTATION_H
#define AUTOROTATION_H

#include <QVariant>
#include <QObject>
#include <libudev.h>
#include <QTimer>

class Daemon;

class AutoRotation : public QObject
{
    Q_OBJECT
    public:
        explicit AutoRotation(Daemon *parent = nullptr);

        void message(QString name, QVariantList args = QVariantList());

    signals:

    public slots:
        void checkRotation();
        void remapTouchScreens();

    private:
        udev* context;
        QString accelerometerPath;
        QString oldOrientation;
        QString primaryDisplay;

        QTimer* timer;
        Daemon* daemon;

        uint switchId = -1;
};

#endif // AUTOROTATION_H
