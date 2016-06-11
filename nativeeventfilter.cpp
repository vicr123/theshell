#include "nativeeventfilter.h"

NativeEventFilter::NativeEventFilter(QObject* parent) : QObject(parent)
{

}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_CLIENT_MESSAGE || event->response_type == (XCB_CLIENT_MESSAGE | 128)) {
            xcb_client_message_event_t* client = static_cast<xcb_client_message_event_t*>(message);

            xcb_atom_t type = client->type;
            std::string name = "_NET_SYSTEM_TRAY_OPCODE";
            xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(QX11Info::connection(), xcb_intern_atom(QX11Info::connection(), 1, name.size(), name.c_str()), nullptr);
            type = reply ? reply->atom : XCB_NONE;
            free(reply);

            if (client->type == type) {
                emit SysTrayEvent(client->data.data32[1], client->data.data32[2], client->data.data32[3], client->data.data32[4]);
            }
        }
    }
    return false;
}
