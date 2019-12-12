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

/****************************************
 *
 *   Parts of this file are adapted from source code of KWin, whose code
 *   is licensed under the GPL (version 2 or later). The copyright message
 *   can be found below:
 *
 *   KWin - the KDE window manager
 *
 *   Copyright (C) 2016 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
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

#include "nativeeventfilter.h"

#include "mainwindow.h"
#include "menu.h"

#include "soundengine.h"

#include <X11/XF86keysym.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

#define Bool int
#define Status int
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI2proto.h>

extern void EndSession(EndSessionWait::shutdownType type);
extern DbusEvents* DBusEvents;
extern MainWindow* MainWin;
extern AudioManager* AudioMan;
extern ScreenRecorder* screenRecorder;


class GeEventMemMover
{
public:
    GeEventMemMover(xcb_generic_event_t *event)
        : m_event(reinterpret_cast<xcb_ge_generic_event_t *>(event))
    {
        // xcb event structs contain stuff that wasn't on the wire, the full_sequence field
        // adds an extra 4 bytes and generic events cookie data is on the wire right after the standard 32 bytes.
        // Move this data back to have the same layout in memory as it was on the wire
        // and allow casting, overwriting the full_sequence field.
        memmove((char*) m_event + 32, (char*) m_event + 36, m_event->length * 4);
    }
    ~GeEventMemMover()
    {
        // move memory layout back, so that Qt can do the same without breaking
        memmove((char*) m_event + 36, (char *) m_event + 32, m_event->length * 4);
    }

    xcb_ge_generic_event_t *operator->() const {
        return m_event;
    }

private:
    xcb_ge_generic_event_t *m_event;
};

static inline qreal fixed1616ToReal(FP1616 val)
{
    return (val) * 1.0 / (1 << 16);
}

struct NativeEventFilterPrivate {
    enum TouchTrackingType {
        None,
        GatewayOpen
    };

    QTime lastPress;

    bool isEndSessionBoxShowing = false;

    QSettings settings;
    QSettings* themeSettings = new QSettings("theSuite", "ts-qtplatform");

    QTimer* powerButtonTimer = nullptr;
    bool powerPressed = false;

    int systrayOpcode, xiOpcode;
    int touchTracking = 0;
    TouchTrackingType touchTrackingType;

    QHash<uint32_t, QPointF> firstTouchPoints;
    QVector<QPointF> touchPoints;
};

NativeEventFilter::NativeEventFilter(QObject* parent) : QObject(parent)
{
    d = new NativeEventFilterPrivate();

    d->powerButtonTimer = new QTimer();
    d->powerButtonTimer->setInterval(500);
    d->powerButtonTimer->setSingleShot(true);
    connect(d->powerButtonTimer, SIGNAL(timeout()), this, SLOT(handlePowerButton()));

    //Capture required keys
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Num_Lock), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Caps_Lock), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);

    //Check if the user wants to capture the super key
    if (d->settings.value("input/superkeyGateway", true).toBool()) {
        XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_L), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
        XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_R), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    }

    //Start the Last Pressed timer to ignore repeated keys
    d->lastPress.start();

    //Get all opcodes needed
    //Get the System Tray opcode
    std::string name = "_NET_SYSTEM_TRAY_OPCODE";
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(QX11Info::connection(), xcb_intern_atom(QX11Info::connection(), 1, name.size(), name.c_str()), nullptr);
    d->systrayOpcode = reply ? reply->atom : XCB_NONE;
    free(reply);

    //Get the XInput opcode
    bool initXinput = true;
    int event, error;
    if (!XQueryExtension(QX11Info::display(), "XInputExtension", &d->xiOpcode, &event, &error)) {
        initXinput = false;
    }

    if (initXinput) {
        //Capture the touch screen
        XIEventMask masks[1];
        unsigned char mask1[XIMaskLen(XI_LASTEVENT)];

        memset(mask1, 0, sizeof(mask1));

        XISetMask(mask1, XI_TouchBegin);
        XISetMask(mask1, XI_TouchUpdate);
        XISetMask(mask1, XI_TouchEnd);
        XISetMask(mask1, XI_TouchOwnership);

        masks[0].deviceid = XIAllDevices;
        masks[0].mask_len = sizeof(mask1);
        masks[0].mask = mask1;

        XSetErrorHandler([](Display* d, XErrorEvent* e) {
            qDebug() << "X11 error:" << e->error_code;
            return 0;
        });
        XISelectEvents(QX11Info::display(), QX11Info::appRootWindow(), masks, 1);
    }
}

NativeEventFilter::~NativeEventFilter() {
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_L), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_R), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Num_Lock), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Caps_Lock), AnyModifier, QX11Info::appRootWindow());

    delete d;
}


bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(result)

    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_CLIENT_MESSAGE || event->response_type == (XCB_CLIENT_MESSAGE | 128)) { //System Tray Event
            //Get the message
            xcb_client_message_event_t* client = static_cast<xcb_client_message_event_t*>(message);

            if (client->type == d->systrayOpcode) {
                //Dock the system tray
                emit SysTrayEvent(client->data.data32[1], client->data.data32[2], client->data.data32[3], client->data.data32[4]);
            }
        } else if (event->response_type == XCB_GE_GENERIC) {
            GeEventMemMover ge(event);
            if (ge->extension == d->xiOpcode) {
                switch (ge->event_type) {
                    case XI_TouchBegin: {
                        xXIDeviceEvent* dEvent = reinterpret_cast<xXIDeviceEvent*>(event);

                        d->firstTouchPoints.insert(dEvent->detail, QPointF(fixed1616ToReal(dEvent->event_x), fixed1616ToReal(dEvent->event_y)));
                        break;
                    }
                    case XI_TouchUpdate: {
                        xXIDeviceEvent* dEvent = reinterpret_cast<xXIDeviceEvent*>(event);
                        if (d->touchTracking == dEvent->detail) {
                            switch (d->touchTrackingType) {
                                case NativeEventFilterPrivate::GatewayOpen:
                                    MainWin->getMenu()->showPartial(fixed1616ToReal(dEvent->event_x));
                                    break;
                            }

                            d->touchPoints.prepend(QPointF(fixed1616ToReal(dEvent->event_x), fixed1616ToReal(dEvent->event_y)));
                            if (d->touchPoints.count() > 10) d->touchPoints.removeLast();
                        }
                        break;
                    }
                    case XI_TouchEnd: {
                        xXIDeviceEvent* dEvent = reinterpret_cast<xXIDeviceEvent*>(event);

                        if (d->touchTracking == dEvent->detail) {
                            switch (d->touchTrackingType) {
                                case NativeEventFilterPrivate::GatewayOpen:
                                    if (!d->touchPoints.isEmpty() && d->touchPoints.last().x() < fixed1616ToReal(dEvent->event_x)) {
                                        MainWin->getMenu()->show();
                                    } else {
                                        MainWin->getMenu()->close();
                                    }
                            }

                            d->touchTracking = 0;
                            d->touchTrackingType = NativeEventFilterPrivate::None;
                        }
                        d->firstTouchPoints.remove(dEvent->detail);
                        break;
                    }
                    case XI_TouchOwnership: {
                        d->touchPoints.clear();

                        xXITouchOwnershipEvent* dEvent = reinterpret_cast<xXITouchOwnershipEvent*>(event);
                        if (d->firstTouchPoints.contains(dEvent->touchid)) {
                            QRect screenGeometry = QApplication::screens().first()->geometry();
                            QPointF point = d->firstTouchPoints.value(dEvent->touchid);
                            if (point.x() >= screenGeometry.x() && point.x() < screenGeometry.x() + 20 && !MainWin->getMenu()->isVisible() && d->settings.value("gestures/swipeGateway", true).toBool()) {
                                //Open the Gateway
                                d->touchTracking = dEvent->touchid;
                                d->touchTrackingType = NativeEventFilterPrivate::GatewayOpen;
                                XIAllowTouchEvents(QX11Info::display(), dEvent->deviceid, dEvent->touchid, QX11Info::appRootWindow(), XIAcceptTouch);
                                MainWin->getMenu()->prepareForShow();
                                return true;
                            }
                        }

                        //We don't know what to do with this, so reject it immediately
                        XIAllowTouchEvents(QX11Info::display(), dEvent->deviceid, dEvent->touchid, QX11Info::appRootWindow(), XIRejectTouch);
                        break;
                    }
                }
            }
        } else if (event->response_type == XCB_KEY_RELEASE) {
            xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);
            if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_Num_Lock) || button->detail == XKeysymToKeycode(QX11Info::display(), XK_Caps_Lock)) {
                if (d->themeSettings->value("accessibility/bellOnCapsNumLock", false).toBool()) {
                    QSoundEffect* sound = new QSoundEffect();
                    sound->setSource(QUrl("qrc:/sounds/keylocks.wav"));
                    sound->play();
                    connect(sound, SIGNAL(playingChanged()), sound, SLOT(deleteLater()));
                }
            }
        }
    }
    return false;
}

void NativeEventFilter::handlePowerButton() {
    if (!d->isEndSessionBoxShowing && !d->powerPressed) {
        //Perform an action depending on what the user wants
        switch (d->settings.value("power/onPowerButtonPressed", 0).toInt()) {
            case 0: { //Ask what to do
                d->isEndSessionBoxShowing = true;

                EndSessionWait* endSession;
                if (d->settings.value("input/touch", false).toBool()) {
                    endSession = new EndSessionWait(EndSessionWait::slideOff);
                } else {
                    endSession = new EndSessionWait(EndSessionWait::ask);
                }
                endSession->showFullScreen();
                endSession->exec();

                d->isEndSessionBoxShowing = false;
                break;
            }
            case 1: { //Power Off
                EndSession(EndSessionWait::powerOff);
                break;
            }
            case 2: { //Reboot
                EndSession(EndSessionWait::reboot);
                break;
            }
            case 3: { //Log Out
                EndSession(EndSessionWait::logout);
                break;
            }
            case 4: { //Suspend
                EndSession(EndSessionWait::suspend);
                break;
            }
            case 5: { //Lock
                DBusEvents->LockScreen();
                break;
            }
            case 6: { //Turn off screen
                EndSession(EndSessionWait::screenOff);
                break;
            }
            case 7: { //Hibernate
                EndSession(EndSessionWait::hibernate);
                break;
            }
        }
    } else {
        d->isEndSessionBoxShowing = true;

        EndSessionWait* endSession;
        if (d->settings.value("input/touch", false).toBool()) {
            endSession = new EndSessionWait(EndSessionWait::slideOff);
        } else {
            endSession = new EndSessionWait(EndSessionWait::ask);
        }
        endSession->showFullScreen();
        endSession->exec();

        d->isEndSessionBoxShowing = false;
    }
}
