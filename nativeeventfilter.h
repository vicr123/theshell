#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <QX11Info>

class NativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit NativeEventFilter(QObject* parent = 0);

signals:
    void SysTrayEvent(long opcode, long data2, long data3, long data4);
public slots:

private:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};

#endif // NATIVEEVENTFILTER_H
