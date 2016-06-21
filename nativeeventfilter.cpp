#include "nativeeventfilter.h"

extern void EndSession(EndSessionWait::shutdownType type);
extern DbusEvents* DBusEvents;

NativeEventFilter::NativeEventFilter(QObject* parent) : QObject(parent)
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Inhibit");
    message.setArguments(QList<QVariant>() << "handle-power-key" << "theShell" << "theShell Handles Hardware Power Keys" << "block");
    QDBusReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    powerInhibit = reply;

    Hotkeys = new HotkeyHud();
    Hotkeys->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    Hotkeys->setAttribute(Qt::WA_ShowWithoutActivating, true);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessUp), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessDown), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioLowerVolume), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioRaiseVolume), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioMute), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Eject), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_PowerOff), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Sleep), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Delete), ControlMask | Mod1Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_L), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);

    lastPress.start();
}

NativeEventFilter::~NativeEventFilter() {
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessUp), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessDown), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioLowerVolume), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioRaiseVolume), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioMute), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Eject), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_PowerOff), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Sleep), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Delete), ControlMask | Mod1Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_L), Mod4Mask, QX11Info::appRootWindow());
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
        } else if (event->response_type == XCB_KEY_PRESS) {
            if (lastPress.restart() > 100) {
                xcb_key_press_event_t* button = static_cast<xcb_key_press_event_t*>(message);

                //Get Current Brightness
                QProcess* backlight = new QProcess(this);
                backlight->start("xbacklight -get");
                backlight->waitForFinished();
                float currentBrightness = ceil(QString(backlight->readAll()).toFloat());
                delete backlight;

                //Get Current Volume
                QProcess* mixer = new QProcess(this);
                mixer->start("amixer");
                mixer->waitForFinished();
                QString output(mixer->readAll());
                delete mixer;

                int volume;
                int limit;
                bool readLine = false;
                for (QString line : output.split("\n")) {
                    if (line.startsWith(" ") && readLine) {
                        if (line.startsWith("  Front Left:")) {
                            if (line.contains("[off]")) {
                                volume = 0;
                            } else {
                                QString percent = line.mid(line.indexOf("\[") + 1, 3).remove("\%").remove("]");
                                volume = percent.toInt();
                            }
                        } else if (line.startsWith("  Limits:")) {
                            limit = line.split(" ").last().toInt();
                        }
                    } else {
                        if (line.contains("'Master'")) {
                            readLine = true;
                        } else {
                            readLine = false;
                        }
                    }
                }


                if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessUp)) { //Increase brightness by 10%
                    currentBrightness = currentBrightness + 10;
                    if (currentBrightness > 100) currentBrightness = 100;

                    QProcess* backlightAdj = new QProcess(this);
                    backlightAdj->start("xbacklight -set " + QString::number(currentBrightness));
                    connect(backlightAdj, SIGNAL(finished(int)), backlightAdj, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("video-display"), "Brightness", (int) currentBrightness);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessDown)) { //Decrease brightness by 10%
                    currentBrightness = currentBrightness - 10;
                    if (currentBrightness < 0) currentBrightness = 0;

                    QProcess* backlightAdj = new QProcess(this);
                    backlightAdj->start("xbacklight -set " + QString::number(currentBrightness));
                    connect(backlightAdj, SIGNAL(finished(int)), backlightAdj, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("video-display"), "Brightness", (int) currentBrightness);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioRaiseVolume)) { //Increase Volume by 5%
                    volume = volume + 5;
                    if (volume > 100) volume = 100;
                    QProcess* volumeAdj = new QProcess(this);
                    volumeAdj->start("amixer set Master " + QString::number(limit * (volume / (float) 100)) + " on");
                    connect(volumeAdj, SIGNAL(finished(int)), volumeAdj, SLOT(deleteLater()));

                    QSoundEffect* volumeSound = new QSoundEffect();
                    volumeSound->setSource(QUrl("qrc:/sounds/volfeedback.wav"));
                    volumeSound->play();
                    connect(volumeSound, SIGNAL(playingChanged()), volumeSound, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("audio-volume-high"), "Volume", volume);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioLowerVolume)) { //Decrease Volume by 5%
                    volume = volume - 5;
                    if (volume < 0) volume = 0;
                    QProcess* volumeAdj = new QProcess(this);
                    volumeAdj->start("amixer set Master " + QString::number(limit * (volume / (float) 100)) + " on");
                    connect(volumeAdj, SIGNAL(finished(int)), volumeAdj, SLOT(deleteLater()));

                    QSoundEffect* volumeSound = new QSoundEffect();
                    volumeSound->setSource(QUrl("qrc:/sounds/volfeedback.wav"));
                    volumeSound->play();
                    connect(volumeSound, SIGNAL(playingChanged()), volumeSound, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("audio-volume-high"), "Volume", volume);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioMute)) { //Set Volume to 0%
                    volume = 0;
                    QProcess* volumeAdj = new QProcess(this);
                    volumeAdj->start("amixer set Master off");
                    connect(volumeAdj, SIGNAL(finished(int)), volumeAdj, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("audio-volume-high"), "Volume", volume);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_Eject)) { //Eject Disc
                    QProcess* eject = new QProcess(this);
                    eject->start("eject");
                    connect(eject, SIGNAL(finished(int)), eject, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("media-eject"), "Eject", "Attempting to eject disc...");
                } else if ((button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_PowerOff)) ||
                           button->detail == XKeysymToKeycode(QX11Info::display(), XK_Delete) && button->state == ControlMask | Mod1Mask) { //Power Off
                    if (!isEndSessionBoxShowing) {
                        isEndSessionBoxShowing = true;
                        /*if (QMessageBox::question(Hotkeys, "Power Off", "Are you sure you wish to close all applications and power off the computer?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
                            EndSession(EndSessionWait::powerOff);
                        }*/

                        EndSessionWait* endSession = new EndSessionWait(EndSessionWait::ask);
                        endSession->showFullScreen();
                        endSession->exec();
                        isEndSessionBoxShowing = false;
                    }

                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_Sleep)) { //Suspend
                    QList<QVariant> arguments;
                    arguments.append(true);

                    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Suspend");
                    message.setArguments(arguments);
                    QDBusConnection::systemBus().send(message);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_L) && button->state == Mod4Mask) { //Lock Screen
                    DBusEvents->LockScreen();
                }
            }
        }
    }
    return false;
}
