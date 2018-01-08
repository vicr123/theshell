/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#include "taskbarmanager.h"

TaskbarManager::TaskbarManager(QObject *parent) : QObject(parent)
{
    //XSelectInput(QX11Info::display(), DefaultRootWindow(QX11Info::display()), PropertyChangeMask | SubstructureNotifyMask);
}

void TaskbarManager::ReloadWindows() {
    /*Window rootReturn, parentReturn, *childrenReturn;
    unsigned int nchildren;

    XQueryTree(QX11Info::display(), DefaultRootWindow(QX11Info::display()), &rootReturn, &parentReturn, &childrenReturn, &nchildren);

    for (unsigned int i = 0; i < nchildren; i++) {
        updateInternalWindow(childrenReturn[i]);
    }
    XFree(childrenReturn);*/

    QList<Window> lostWindows = knownWindows.keys();

    Atom WindowListType;
    int format;
    unsigned long items, bytes;
    unsigned char *data;
    XGetWindowProperty(QX11Info::display(), DefaultRootWindow(QX11Info::display()), XInternAtom(QX11Info::display(), "_NET_CLIENT_LIST", true), 0L, (~0L),
                                    False, AnyPropertyType, &WindowListType, &format, &items, &bytes, &data);

    quint64 *windows = (quint64*) data;
    for (unsigned int i = 0; i < items; i++) {
        if (updateInternalWindow(windows[i])) {
            lostWindows.removeAll(windows[i]);
        }
    }
    XFree(data);

    for (Window window : lostWindows) {
        emit deleteWindow(knownWindows.value(window));
        knownWindows.remove(window);
    }
}

bool TaskbarManager::updateInternalWindow(Window window) {
    WmWindow serialised;

    int currentDesktop = 0;
    { //Get the current desktop
        unsigned long *desktop;
        unsigned long items, bytes;
        int format;
        Atom ReturnType;

        int retval = XGetWindowProperty(QX11Info::display(), DefaultRootWindow(QX11Info::display()), XInternAtom(QX11Info::display(), "_NET_CURRENT_DESKTOP", False), 0, 1024, False,
                                        XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &desktop);
        if (retval == 0 && desktop != 0) {
            currentDesktop = *desktop;
        }
        XFree(desktop);
    }

    int ok;
    unsigned long items, bytes;
    unsigned char *returnVal;
    int format;
    Atom ReturnType;

    //Query Title
    ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "_NET_WM_NAME", False), 0, 1024, False,
                       XInternAtom(QX11Info::display(), "UTF8_STRING", False), &ReturnType, &format, &items, &bytes, &returnVal);

    if (returnVal == 0x0) {
        ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "WM_NAME", False), 0, 1024, False,
                           XInternAtom(QX11Info::display(), "UTF8_STRING", False), &ReturnType, &format, &items, &bytes, &returnVal);
    }

    if (ok == 0 && returnVal != 0x0) {
        serialised.setTitle(QString::fromUtf8((char*) returnVal));
        XFree(returnVal);

        ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "_NET_WM_PID", False), 0, 1024, False,
                                XA_CARDINAL, &ReturnType, &format, &items, &bytes, &returnVal);
        if (ok == 0 && returnVal != 0x0) {
            unsigned long pid = *(unsigned long*) returnVal;
            serialised.setPID(pid);

            XFree(returnVal);
        }

        {
            bool noIcon = false;
            int width, height;

            //Get all icon sizes
            //QMap<int, QSize> iconSizes;

            //do {
                ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "_NET_WM_ICON", False), 0, 1, False,
                                   XA_CARDINAL, &ReturnType, &format, &items, &bytes, &returnVal);
                if (returnVal == 0x0) {
                    noIcon = true;
                } else {
                    width = *(int*) returnVal;
                    XFree(returnVal);
                }

                ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "_NET_WM_ICON", False), 1, 1, False,
                                   XA_CARDINAL, &ReturnType, &format, &items, &bytes, &returnVal);

                if (returnVal == 0x0) {
                    noIcon = true;
                } else {
                    height = *(int*) returnVal;
                    XFree(returnVal);
                }

                //iconSizes.insert(offset, QSize(width, height));
                //offset += width * height * 4 + 2;
            //} while (!noIcon || icBytes == width * height * 4);

            /*QSize currentSize = QSize(0, 0);

            for (int offsets : iconSizes.keys()) {
                if (currentSize == QSize(0, 0) || (currentSize.width() > iconSizes.value(offsets).width() && currentSize.height() > iconSizes.value(offsets).height())) {
                    currentSize = iconSizes.value(offsets);
                    imageOffset = offsets;
                }
            }*/

            //width = currentSize.width();
            //height = currentSize.height();

            if (!noIcon) {
                ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "_NET_WM_ICON", False), 2, width * height * 4, False,
                                   XA_CARDINAL, &ReturnType, &format, &items, &bytes, &returnVal);

                QImage image(16, 16, QImage::Format_ARGB32);

                float widthSpacing = (float) width / (float) 16;
                float heightSpacing = (float) height / (float) 16;

                for (int y = 0; y < 16; y++) {
                    for (int x = 0; x < 16 * 8; x = x + 8) {
                        unsigned long a, r, g, b;

                        b = (returnVal[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 0)]);
                        g = (returnVal[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 1)]);
                        r = (returnVal[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 2)]);
                        a = (returnVal[(int) (y * heightSpacing * width * 8 + x * widthSpacing + 3)]);

                        QColor col = QColor(r, g, b, a);

                        image.setPixelColor(x / 8, y, col);
                    }
                }

                QPixmap iconPixmap(QPixmap::fromImage(image).scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
                serialised.setIcon(QIcon(iconPixmap));

                XFree(returnVal);
            }
        }

        ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "_NET_WM_STATE", False), 0, 1024, False,
                               XA_ATOM, &ReturnType, &format, &items, &bytes, (unsigned char**) &returnVal);

        {
            Atom* atoms = (Atom*) returnVal;
            for (unsigned int i = 0; i < items; i++) {
                if (atoms[i] == XInternAtom(QX11Info::display(), "_NET_WM_STATE_HIDDEN", False)) {
                    serialised.setMinimized(true);
                } else if (atoms[i] == XInternAtom(QX11Info::display(), "_NET_WM_STATE_SKIP_TASKBAR", False)) {
                    //skipTaskbar = true;
                } else if (atoms[i] == XInternAtom(QX11Info::display(), "_NET_WM_STATE_DEMANDS_ATTENTION", False)) {
                    serialised.setAttention(true);
                }
            }
        }

        XFree(returnVal);

        {
            ok = XGetWindowProperty(QX11Info::display(), window, XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False), 0, 1024, False,
                                            XA_CARDINAL, &ReturnType, &format, &items, &bytes, (unsigned char**) &returnVal);
            if (ok == 0 && returnVal != 0) {
                serialised.setDesktop(*returnVal);
            }
            XFree(returnVal);
        }


        XWindowAttributes attributes;
        ok = XGetWindowAttributes(QX11Info::display(), window, &attributes);

        int windowx, windowy;
        Window child;
        ok = XTranslateCoordinates(QX11Info::display(), window, RootWindow(QX11Info::display(), 0), 0, 0, &windowx, &windowy, &child);

        serialised.setGeometry(QRect(windowx, windowy, attributes.width, attributes.height));

        serialised.setWID(window);

        if (serialised.PID() == QApplication::applicationPid() && serialised.title() != "Choose Background") {
            //theShell window. Ignore.
            return false;
        } else {
            if (settings.value("bar/showWindowsFromOtherDesktops", true).toBool() ||
                             serialised.desktop() == currentDesktop) {
                knownWindows.insert(window, serialised);
                emit updateWindow(serialised);
                return true;
            } else {
                //Window not on current desktop. Ignore.
                return false;
            }
        }
    } else {
        //Invalid window. Ignore.
        return false;
    }
    return false;
}

QList<WmWindow> TaskbarManager::Windows() {
    return knownWindows.values();
}
