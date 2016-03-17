#include "nativeeventfilter.h"

NativeEventFilter::NativeEventFilter()
{

}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    /*xcb_generic_event_t* msg = (xcb_generic_event_t*) message;
    XEvent *event;
    XNextEvent(QX11Info::display(), event);

    if (event->type == KeyPress) {
        XKeyPressedEvent *keyEvent = (XKeyPressedEvent*) event;
        KeyCode code = keyEvent->keycode;

    }*/

    return false;
}
