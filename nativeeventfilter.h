#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QAbstractNativeEventFilter>
#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <QX11Info>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

class NativeEventFilter : public QAbstractNativeEventFilter
{
public:
    NativeEventFilter();

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};

#endif // NATIVEEVENTFILTER_H
