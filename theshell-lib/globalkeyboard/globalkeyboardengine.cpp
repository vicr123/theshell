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
#include "globalkeyboardengine.h"

#include <QKeySequence>
#include <QMap>
#include <QCoreApplication>
#include <QX11Info>

#include <QDebug>
#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <xcb/xcb.h>

#include "keyboardtables.h"

struct GlobalKeyboardEnginePrivate {
    GlobalKeyboardEngine* instance = nullptr;
    QMap<QKeySequence, GlobalKeyboardKey*> keyMapping;

    int currentChordNumber = 0;
    QList<GlobalKeyboardKey*> chordingKeys;

    int listening = 0;
};

GlobalKeyboardEnginePrivate* GlobalKeyboardEngine::d = new GlobalKeyboardEnginePrivate();

GlobalKeyboardEngine::GlobalKeyboardEngine(QObject *parent) : QObject(parent)
{
    QCoreApplication::instance()->installNativeEventFilter(this);
}

GlobalKeyboardKey* GlobalKeyboardEngine::registerKey(QKeySequence keySequence, QString name) {
    //Initialise an instance first
    GlobalKeyboardEngine::instance();

    if (d->keyMapping.contains(keySequence)) return nullptr; //Don't allow conflicting keys

    GlobalKeyboardKey* key = new GlobalKeyboardKey(keySequence);
    d->keyMapping.insert(keySequence, key);
    connect(key, &GlobalKeyboardKey::deregistered, [=] {
        d->keyMapping.remove(keySequence);
        key->deleteLater();
    });
    emit d->instance->keyShortcutRegistered(name, key);
    return key;
}

GlobalKeyboardEngine* GlobalKeyboardEngine::instance() {
    if (d->instance == nullptr) d->instance = new GlobalKeyboardEngine();
    return d->instance;
}

bool GlobalKeyboardEngine::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(result)

    if (d->listening != 0) return false;
    xcb_generic_event_t* event = static_cast<xcb_generic_event_t*>(message);
    if (event->response_type == XCB_KEY_PRESS) { //Key Press Event
        xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);
        ulong keyState = 0;
        if (button->state & XCB_MOD_MASK_1) keyState |= Mod1Mask;
        if (button->state & XCB_MOD_MASK_CONTROL) keyState |= ControlMask;
        if (button->state & XCB_MOD_MASK_4) keyState |= Mod4Mask;
        if (button->state & XCB_MOD_MASK_SHIFT) keyState |= ShiftMask;

        if (d->currentChordNumber == 0) {
            bool beginChording = false;
            QList<GlobalKeyboardKey*> matchingKeys;
            for (GlobalKeyboardKey* key : d->keyMapping.values()) {
                if (XKeysymToKeycode(QX11Info::display(), key->nativeKey(0)) == button->detail && key->nativeModifiers(0) == keyState) {
                    matchingKeys.append(key);
                    if (key->chordCount() > 1) beginChording = true;
                }
            }

            if (beginChording) {
                d->chordingKeys = matchingKeys;
                d->currentChordNumber = 1;
                XGrabKeyboard(QX11Info::display(), QX11Info::appRootWindow(), true, GrabModeAsync, GrabModeAsync, CurrentTime);
            } else {
                if (matchingKeys.count() == 1) {
                    emit matchingKeys.first()->shortcutActivated();
                    return true;
                } else if (matchingKeys.count() > 1) {
                    //Conflict!!!!!!!
                    qDebug() << "Key conflict!";
                    return true;
                }
            }
        } else {
            //Go through all the keys that need to be checked for chords
            QList<GlobalKeyboardKey*> matchingKeys;
            for (GlobalKeyboardKey* key : d->chordingKeys) {
                if (XKeysymToKeycode(QX11Info::display(), key->nativeKey(d->currentChordNumber)) == button->detail && key->nativeModifiers(d->currentChordNumber) == keyState) {
                    matchingKeys.append(key);
                }
            }

            if (matchingKeys.count() == 1) {
                emit matchingKeys.first()->shortcutActivated();
                XUngrabKeyboard(QX11Info::display(), CurrentTime);
                d->currentChordNumber = 0;
                return true;
            } else if (matchingKeys.count() == 0) {
                //No keys matched chord
                XUngrabKeyboard(QX11Info::display(), CurrentTime);
                d->currentChordNumber = 0;
                return true;
            } else {
                d->chordingKeys = matchingKeys;
                d->currentChordNumber++;

                if (d->currentChordNumber == 4) {
                    //Conflict!
                    XUngrabKeyboard(QX11Info::display(), CurrentTime);
                    d->currentChordNumber = 0;
                    return true;
                }
            }
        }
    }
    return false;
}

void GlobalKeyboardEngine::startListening() {
    d->listening--;
    if (d->listening == 0) {
        for (GlobalKeyboardKey* key : d->keyMapping.values()) {
            key->grabKey();
        }
    }
}

void GlobalKeyboardEngine::pauseListening() {
    d->listening++;
    if (d->listening == 1) {
        for (GlobalKeyboardKey* key : d->keyMapping.values()) {
            key->ungrabKey();
        }
    }
}

QString GlobalKeyboardEngine::keyName(KnownKeyNames name) {
    switch (name) {
        case BrightnessUp:
            return "Dsp-BrightnessUp";
        case BrightnessDown:
            return "Dsp-BrightnessDown";
        case VolumeUp:
            return "Audio-VolumeUp";
        case VolumeDown:
            return "Audio-VolumeDown";
        default:
            return "";
    }
}

struct GlobalKeyboardKeyPrivate {
    QKeySequence key;

    QString section;
    QString description;

    bool grabbed = false;

    static QMap<KeyCode, int> grabbedKeycodes;
};

QMap<KeyCode, int> GlobalKeyboardKeyPrivate::grabbedKeycodes;

GlobalKeyboardKey::GlobalKeyboardKey(QKeySequence key, QString section, QString description, QObject* parent) : QObject(parent) {
    d = new GlobalKeyboardKeyPrivate();
    d->key = key;
    d->section = section;
    d->description = description;

    grabKey();
}

GlobalKeyboardKey::~GlobalKeyboardKey() {
    ungrabKey();
    delete d;
}

void GlobalKeyboardKey::grabKey() {
    if (d->grabbed) return;
    //Grab this key
    KeyCode kc = XKeysymToKeycode(QX11Info::display(), nativeKey(0));
    if (d->grabbedKeycodes.contains(kc)) {
        d->grabbedKeycodes.insert(kc, d->grabbedKeycodes.value(kc) + 1);
    } else {
        d->grabbedKeycodes.insert(kc, 1);
        XGrabKey(QX11Info::display(), kc, nativeModifiers(0), QX11Info::appRootWindow(), true, GrabModeAsync, GrabModeAsync);
    }
    d->grabbed = true;
}

void GlobalKeyboardKey::ungrabKey() {
    if (!d->grabbed) return;
    //Ungrab this key
    KeyCode kc = XKeysymToKeycode(QX11Info::display(), nativeKey(0));
    if (d->grabbedKeycodes.value(kc) == 1) {
        XUngrabKey(QX11Info::display(), kc, nativeModifiers(0), QX11Info::appRootWindow());
        d->grabbedKeycodes.remove(kc);
    } else {
        d->grabbedKeycodes.insert(kc, d->grabbedKeycodes.value(kc) - 1);
    }
    d->grabbed = false;
}

unsigned long GlobalKeyboardKey::nativeKey(uint chordNumber) {
    uint keycode = static_cast<uint>(d->key[chordNumber]) & ~(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier);
    return toKeySym(Qt::Key(keycode));
}

unsigned long GlobalKeyboardKey::nativeModifiers(uint chordNumber) {
    return toNativeModifiers(Qt::KeyboardModifiers(static_cast<uint>(d->key[chordNumber]) & (Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)));
}

int GlobalKeyboardKey::chordCount() {
    return d->key.count();
}

QString GlobalKeyboardKey::section() {
    return d->section;
}

QString GlobalKeyboardKey::description() {
    return d->description;
}

void GlobalKeyboardKey::deregister() {
    emit deregistered();
}
