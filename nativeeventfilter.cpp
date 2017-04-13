#include "nativeeventfilter.h"

extern void EndSession(EndSessionWait::shutdownType type);
extern DbusEvents* DBusEvents;
extern MainWindow* MainWin;
extern AudioManager* AudioMan;

NativeEventFilter::NativeEventFilter(QObject* parent) : QObject(parent)
{
    //Create the Hotkey window and set appropriate flags
    Hotkeys = new HotkeyHud();
    Hotkeys->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    Hotkeys->setAttribute(Qt::WA_ShowWithoutActivating, true);

    //Capture required keys
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessUp), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessDown), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_KbdBrightnessUp), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_KbdBrightnessDown), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioLowerVolume), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioRaiseVolume), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioMute), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Eject), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_PowerOff), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Sleep), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Print), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Delete), ControlMask | Mod1Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_L), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F2), Mod1Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_space), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_P), Mod4Mask | Mod1Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F1), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F2), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F3), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F4), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F5), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F6), Mod4Mask, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);


    //Check if the user wants to capture the super key
    if (settings.value("input/superkeyGateway", true).toBool()) {
        XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_L), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
        XGrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_R), AnyModifier, RootWindow(QX11Info::display(), 0), true, GrabModeAsync, GrabModeAsync);
    }

    //Start the Last Pressed timer to ignore repeated keys
    lastPress.start();
}

NativeEventFilter::~NativeEventFilter() {
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessUp), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessDown), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_KbdBrightnessUp), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_KbdBrightnessDown), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioLowerVolume), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioRaiseVolume), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_AudioMute), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Eject), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_PowerOff), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XF86XK_Sleep), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Print), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Delete), ControlMask | Mod1Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_L), Mod4Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F2), Mod1Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_L), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Super_R), AnyModifier, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_space), Mod4Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_P), Mod4Mask | Mod1Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F1), Mod4Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F2), Mod4Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F3), Mod4Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F4), Mod4Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F5), Mod4Mask, QX11Info::appRootWindow());
    XUngrabKey(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_F6), Mod4Mask, QX11Info::appRootWindow());
}


bool NativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
        if (event->response_type == XCB_CLIENT_MESSAGE || event->response_type == (XCB_CLIENT_MESSAGE | 128)) { //System Tray Event
            //Get the message
            xcb_client_message_event_t* client = static_cast<xcb_client_message_event_t*>(message);

            xcb_atom_t type = client->type;
            std::string name = "_NET_SYSTEM_TRAY_OPCODE";
            xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(QX11Info::connection(), xcb_intern_atom(QX11Info::connection(), 1, name.size(), name.c_str()), nullptr);
            type = reply ? reply->atom : XCB_NONE;
            free(reply);

            if (client->type == type) {
                //Dock the system tray
                emit SysTrayEvent(client->data.data32[1], client->data.data32[2], client->data.data32[3], client->data.data32[4]);
            }
        } else if (event->response_type == XCB_KEY_PRESS) { //Key Press Event
            if (lastPress.restart() > 100) {
                xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);

                //Get Current Brightness
                QProcess* backlight = new QProcess(this);
                backlight->start("xbacklight -get");
                backlight->waitForFinished();
                float currentBrightness = ceil(QString(backlight->readAll()).toFloat());
                delete backlight;

                //Get Current Volume
                int volume = AudioMan->MasterVolume();

                int kbdBrightness = -1, maxKbdBrightness = -1;
                QDBusInterface keyboardInterface("org.freedesktop.UPower", "/org/freedesktop/UPower/KbdBacklight", "org.freedesktop.UPower.KbdBacklight", QDBusConnection::systemBus());
                if (keyboardInterface.isValid()) {
                    kbdBrightness = keyboardInterface.call("GetBrightness").arguments().first().toInt();
                    maxKbdBrightness = keyboardInterface.call("GetMaxBrightness").arguments().first().toInt();
                }

                if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessUp)) { //Increase brightness by 10%
                    currentBrightness = currentBrightness + 10;
                    if (currentBrightness > 100) currentBrightness = 100;

                    QProcess* backlightAdj = new QProcess(this);
                    backlightAdj->start("xbacklight -set " + QString::number(currentBrightness));
                    connect(backlightAdj, SIGNAL(finished(int)), backlightAdj, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("video-display"), tr("Brightness"), (int) currentBrightness);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_MonBrightnessDown)) { //Decrease brightness by 10%
                    currentBrightness = currentBrightness - 10;
                    if (currentBrightness < 0) currentBrightness = 0;

                    QProcess* backlightAdj = new QProcess(this);
                    backlightAdj->start("xbacklight -set " + QString::number(currentBrightness));
                    connect(backlightAdj, SIGNAL(finished(int)), backlightAdj, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("video-display"), tr("Brightness"), (int) currentBrightness);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioRaiseVolume)) { //Increase Volume by 5%
                    volume = volume + 5;
                    if (volume - 5 < 100 && volume > 100) {
                        volume = 100;
                    }
                    AudioMan->changeVolume(5);

                    QSoundEffect* volumeSound = new QSoundEffect();
                    volumeSound->setSource(QUrl("qrc:/sounds/volfeedback.wav"));
                    volumeSound->play();
                    connect(volumeSound, SIGNAL(playingChanged()), volumeSound, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("audio-volume-high"), tr("Volume"), volume);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioLowerVolume)) { //Decrease Volume by 5%
                    volume = volume - 5;
                    if (volume < 0) volume = 0;
                    AudioMan->changeVolume(-5);

                    QSoundEffect* volumeSound = new QSoundEffect();
                    volumeSound->setSource(QUrl("qrc:/sounds/volfeedback.wav"));
                    volumeSound->play();
                    connect(volumeSound, SIGNAL(playingChanged()), volumeSound, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("audio-volume-high"), tr("Volume"), volume);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_AudioMute)) { //Set Volume to 0%
                    volume = 0;
                    QProcess* volumeAdj = new QProcess(this);
                    volumeAdj->start("amixer set Master off");
                    connect(volumeAdj, SIGNAL(finished(int)), volumeAdj, SLOT(deleteLater()));

                    Hotkeys->show(QIcon::fromTheme("audio-volume-high"), tr("Volume"), volume);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_KbdBrightnessUp)) { //Increase keyboard brightness by 5%
                    kbdBrightness += (((float) maxKbdBrightness / 100) * 5);
                    if (kbdBrightness > maxKbdBrightness) kbdBrightness = maxKbdBrightness;
                    keyboardInterface.call("SetBrightness", kbdBrightness);

                    Hotkeys->show(QIcon::fromTheme("input-keyboard"), tr("Keyboard Brightness"), ((float) kbdBrightness / (float) maxKbdBrightness) * 100);
                } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_KbdBrightnessDown)) { //Decrease keyboard brightness by 5%
                    kbdBrightness -= (((float) maxKbdBrightness / 100) * 5);
                    if (kbdBrightness < 0) kbdBrightness = 0;
                    keyboardInterface.call("SetBrightness", kbdBrightness);

                    Hotkeys->show(QIcon::fromTheme("input-keyboard"), tr("Keyboard Brightness"), ((float) kbdBrightness / (float) maxKbdBrightness) * 100);
                }
            }
        } else if (event->response_type == XCB_KEY_RELEASE) {
            xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);

            if (button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_Eject)) { //Eject Disc
                QProcess* eject = new QProcess(this);
                eject->start("eject");
                connect(eject, SIGNAL(finished(int)), eject, SLOT(deleteLater()));

                Hotkeys->show(QIcon::fromTheme("media-eject"), tr("Eject"), tr("Attempting to eject disc..."));
            } else if ((button->detail == XKeysymToKeycode(QX11Info::display(), XF86XK_PowerOff)) ||
                       button->detail == XKeysymToKeycode(QX11Info::display(), XK_Delete) && (button->state == (ControlMask | Mod1Mask))) { //Power Off
                if (!isEndSessionBoxShowing) {
                    isEndSessionBoxShowing = true;
                    EndSessionWait* endSession;
                    if (settings.value("input/touch", false).toBool()) {
                        endSession = new EndSessionWait(EndSessionWait::slideOff);
                    } else {
                        endSession = new EndSessionWait(EndSessionWait::ask);
                    }
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
                ignoreSuper = true;
                DBusEvents->LockScreen();
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_F2) && button->state == Mod1Mask) { //Run
                RunDialog* run = new RunDialog();
                run->show();
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_Super_L) || button->detail == XKeysymToKeycode(QX11Info::display(), XK_Super_R)) {
                if (!ignoreSuper) { //Check that the user is not doing a key combination
                    MainWin->openMenu();
                }
                ignoreSuper = false;
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_space) && button->state == Mod4Mask) { //theWave
                ignoreSuper = true;
                MainWin->openMenu(true, true);
            } else if ((button->detail == XKeysymToKeycode(QX11Info::display(), XK_P) && (button->state == (Mod4Mask | Mod1Mask))) ||
                       (button->detail == XKeysymToKeycode(QX11Info::display(), XK_Print))) { //Take screenshot
                if (button->state & Mod4Mask) {
                    ignoreSuper = true;
                }
                screenshotWindow* screenshot = new screenshotWindow;
                screenshot->show();
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_F1) && (button->state == Mod4Mask)) {
                MainWin->getInfoPane()->show(InfoPaneDropdown::Clock);
                ignoreSuper = true;
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_F2) && (button->state == Mod4Mask)) {
                MainWin->getInfoPane()->show(InfoPaneDropdown::Battery);
                ignoreSuper = true;
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_F3) && (button->state == Mod4Mask)) {
                MainWin->getInfoPane()->show(InfoPaneDropdown::Network);
                ignoreSuper = true;
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_F4) && (button->state == Mod4Mask)) {
                MainWin->getInfoPane()->show(InfoPaneDropdown::Notifications);
                ignoreSuper = true;
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_F5) && (button->state == Mod4Mask)) {
                MainWin->getInfoPane()->show(InfoPaneDropdown::KDEConnect);
                ignoreSuper = true;
            } else if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_F6) && (button->state == Mod4Mask)) {
                MainWin->getInfoPane()->show(InfoPaneDropdown::Print);
                ignoreSuper = true;
            }

        }
    }
    return false;
}
