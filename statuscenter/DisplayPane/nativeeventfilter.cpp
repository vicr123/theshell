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
#include "nativeeventfilter.h"

#include <QDebug>
#include <QX11Info>
#include <X11/extensions/Xrandr.h>
#include <xcb/randr.h>

struct NativeEventFilterPrivate {
    DisplayPositionWidget* parent;
    int randrEventBase, randrErrorBase;
};

NativeEventFilter::NativeEventFilter(DisplayPositionWidget *parent) : QObject(parent), QAbstractNativeEventFilter()
{
    d = new NativeEventFilterPrivate();
    d->parent = parent;

    XRRQueryExtension(QX11Info::display(), &d->randrEventBase, &d->randrErrorBase);
}

NativeEventFilter::~NativeEventFilter() {
    delete d;
}

bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(result)

    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == d->randrEventBase + XCB_RANDR_NOTIFY) {
            /*
            if (event->response_type & (d->randrEventBase + XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE) ||
                    event->response_type & (d->randrEventBase + XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE) ||
                    event->response_type & (d->randrEventBase + XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE) ||
                    event->response_type & (d->randrEventBase + XCB_RANDR_NOTIFY_MASK_OUTPUT_PROPERTY)) {*/
                //RandR has changed, update all the displays
                qDebug() << "RandR changed!";
                d->parent->reloadDisplays();
            //}
        }
    }
    return false;
}
