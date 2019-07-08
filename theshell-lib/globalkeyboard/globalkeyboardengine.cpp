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
#include <QPixmap>
#include <QFontMetrics>
#include <QPalette>
#include "shortcutinfodialog.h"
#include <the-libs_global.h>

#include <QDebug>
#include <QPainter>
#include <X11/Xlib.h>
#include <X11/XF86keysym.h>
#include <xcb/xcb.h>
#include <QTimer>

#include "keyboardtables.h"

struct GlobalKeyboardEnginePrivate {
    GlobalKeyboardEngine* instance = nullptr;
    QMap<QKeySequence, GlobalKeyboardKey*> keyMapping;

    int currentChordNumber = 0;
    QList<GlobalKeyboardKey*> chordingKeys;

    ShortcutInfoDialog* shortcutDialog;

    int listening = 0;
    bool heardSuper = false;
};

GlobalKeyboardEnginePrivate* GlobalKeyboardEngine::d = new GlobalKeyboardEnginePrivate();

GlobalKeyboardEngine::GlobalKeyboardEngine(QObject *parent) : QObject(parent)
{
    QCoreApplication::instance()->installNativeEventFilter(this);
    d->shortcutDialog = new ShortcutInfoDialog();

    //Register a new shortcut for the Super key
    QTimer::singleShot(0, [=] {
        registerKey(QKeySequence(Qt::Key_Super_L), keyName(OpenGateway), tr("System"), tr("Open Gateway"), tr("Opens the Gateway"));
    });
}

GlobalKeyboardKey* GlobalKeyboardEngine::registerKey(QKeySequence keySequence, QString name, QString section, QString humanReadableName, QString description) {
    //Initialise an instance first
    GlobalKeyboardEngine::instance();

    if (d->keyMapping.contains(keySequence)) return nullptr; //Don't allow conflicting keys

    GlobalKeyboardKey* key = new GlobalKeyboardKey(keySequence, section, humanReadableName, description);
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
        if (button->state & XCB_MOD_MASK_4) {
            keyState |= Mod4Mask;
            d->heardSuper = true;
        }
        if (button->state & XCB_MOD_MASK_SHIFT) keyState |= ShiftMask;

        for (const int ks : {XK_Control_L, XK_Control_R, XK_Alt_L, XK_Alt_R, XK_Shift_L, XK_Shift_R, XK_Super_L, XK_Super_R, XK_Meta_L, XK_Meta_R, XK_Hyper_L, XK_Hyper_R}) {
            if (button->detail == XKeysymToKeycode(QX11Info::display(), ks)) return false; //Do nothing; this is a modifier key
        }

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
                //d->chordingKeys = matchingKeys;
                //d->currentChordNumber = 1;
                //XGrabKeyboard(QX11Info::display(), QX11Info::appRootWindow(), true, GrabModeAsync, GrabModeAsync, CurrentTime);

                QKeySequence key = matchingKeys.first()->key();
                //d->shortcutDialog->showChords(QKeySequence(key[0]), matchingKeys, tr("Strike the next key in the chord"));
                d->shortcutDialog->showChords(QKeySequence(key[0]), QList<GlobalKeyboardKey*>(), "Chorded shortcuts have been disabled for now");
                QTimer::singleShot(2000, d->shortcutDialog, &ShortcutInfoDialog::hide);
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

            if (matchingKeys.count() == 1 && matchingKeys.first()->chordCount() == d->currentChordNumber + 1) {
                emit matchingKeys.first()->shortcutActivated();
                XUngrabKeyboard(QX11Info::display(), CurrentTime);
                d->currentChordNumber = 0;
                d->shortcutDialog->hide();
                return true;
            } else if (matchingKeys.count() == 0) {
                //No keys matched chord
                XUngrabKeyboard(QX11Info::display(), CurrentTime);
                d->currentChordNumber = 0;
                d->shortcutDialog->hide();
                return true;
            } else {
                d->chordingKeys = matchingKeys;
                d->currentChordNumber++;

                QKeySequence key = matchingKeys.first()->key();
                d->shortcutDialog->showChords(QKeySequence(key[0], key[1], d->currentChordNumber > 2 ? key[2] : -1, d->currentChordNumber > 3 ? key[3] : -1), matchingKeys, tr("Strike the next key in the chord"));

                if (d->currentChordNumber == 4) {
                    //Conflict!
                    XUngrabKeyboard(QX11Info::display(), CurrentTime);
                    d->currentChordNumber = 0;
                    d->shortcutDialog->hide();
                    return true;
                }
            }
        }
    } else if (event->response_type == XCB_KEY_RELEASE) { //Key Release Event
        xcb_key_release_event_t* button = static_cast<xcb_key_release_event_t*>(message);
        if (button->detail == XKeysymToKeycode(QX11Info::display(), XK_Super_L)) {
            if (d->heardSuper) {
                d->heardSuper = false;
            } else {
                emit d->keyMapping.value(QKeySequence(Qt::Key_Super_L))->shortcutActivated();
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

QPixmap GlobalKeyboardEngine::getKeyShortcutImage(QKeySequence keySequence, QFont font, QPalette pal) {
    QFontMetrics metrics(font);
    QString sequence = keySequence.toString();
    QStringList chordParts = sequence.split(", ", QString::SkipEmptyParts);
    if (chordParts.count() == 0) {
        QRect textRect;
        textRect.setWidth(metrics.width(tr("No Shortcut")) + 1);
        textRect.setHeight(metrics.height());
        textRect.moveLeft(0);
        textRect.moveTop(0);

        QPixmap pixmap(textRect.size());
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setPen(pal.color(QPalette::WindowText));
        painter.drawText(textRect, tr("No Shortcut"));

        return pixmap;
    } else {
        int width = 0;
        for (int i = 0; i < chordParts.count(); i++) {
            QStringList keys = chordParts.at(i).split("+", QString::SkipEmptyParts);

            for (QString key : keys) {
                QPixmap icon = getKeyIcon(key, font, pal);
                width += icon.width() + SC_DPI(4);
            }

            if (chordParts.count() > i + 1) {
                width += metrics.width(",") + 1 + SC_DPI(4);
            }
        }
        width -= SC_DPI(4);

        QPixmap pixmap(width, qMax(SC_DPI(24), metrics.height() + SC_DPI(8)));
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(pal.color(QPalette::WindowText));
        int currentX = 0;

        for (int i = 0; i < chordParts.count(); i++) {
            QStringList keys = chordParts.at(i).split("+", QString::SkipEmptyParts);

            for (QString key : keys) {
                QPixmap icon = getKeyIcon(key, font, pal);
                painter.drawPixmap(currentX, SC_DPI(4), icon);
                currentX += icon.width() + SC_DPI(4);
            }

            if (chordParts.count() > i + 1) {
                QRect textRect;
                textRect.setWidth(metrics.width(",") + 1);
                textRect.setHeight(metrics.height());
                textRect.moveLeft(currentX);
                textRect.moveTop(pixmap.height() / 2 - textRect.height() / 2);
                painter.drawText(textRect, ",");
                currentX = textRect.right() + SC_DPI(4);
            }
        }

        return pixmap;
    }
}

QPixmap GlobalKeyboardEngine::getKeyIcon(QString key, QFont font, QPalette pal) {
    //Special Cases
    if (key == "Meta") key = "Super";
    if (key == "Print") key = "PrtSc";

    QPixmap squarePx(SC_DPI_T(QSize(16, 16), QSize));
    squarePx.fill(Qt::transparent);

    QPainter sqPainter(&squarePx);
    sqPainter.setRenderHint(QPainter::Antialiasing);
    sqPainter.setPen(Qt::transparent);
    sqPainter.setBrush(pal.color(QPalette::WindowText));
    sqPainter.drawRoundedRect(QRect(QPoint(0, 0), squarePx.size()), 50, 50, Qt::RelativeSize);

    QRect squareIconRect;
    squareIconRect.setWidth(12 * theLibsGlobal::getDPIScaling());
    squareIconRect.setHeight(12 * theLibsGlobal::getDPIScaling());
    squareIconRect.moveCenter(QPoint(8, 8) * theLibsGlobal::getDPIScaling());

    /*if (key == "Left") {
        QImage image = QIcon::fromTheme("go-previous").pixmap(squareIconRect.size()).toImage();
        tintImage(image, this->palette().color(QPalette::Window));
        sqPainter.drawImage(squareIconRect, image);
        return squarePx;
    } else if (key == "Right") {
        QImage image = QIcon::fromTheme("go-next").pixmap(squareIconRect.size()).toImage();
        tintImage(image, this->palette().color(QPalette::Window));
        sqPainter.drawImage(squareIconRect, image);
        return squarePx;
    } else {*/

        //font.setPointSizeF(floor(8));
        /*while (QFontMetrics(font).height() > 14 * theLibsGlobal::getDPIScaling()) {
            font.setPointSizeF(font.pointSizeF() - 0.5);
        }*/
        font.setPixelSize(SC_DPI(12));
        QFontMetrics fontMetrics(font);

        QSize pixmapSize;
        pixmapSize.setHeight(SC_DPI(16));
        pixmapSize.setWidth(qMax(fontMetrics.width(key) + SC_DPI(6), SC_DPI(16)));

        QPixmap px(pixmapSize);
        px.fill(Qt::transparent);

        QPainter painter(&px);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::transparent);
        painter.setBrush(pal.color(QPalette::WindowText));
        painter.drawRoundedRect(QRect(QPoint(0, 0), px.size()), 4 * theLibsGlobal::getDPIScaling(), 4 * theLibsGlobal::getDPIScaling());

        painter.setFont(font);
        painter.setPen(pal.color(QPalette::Window));

        QRect textRect;
        textRect.setHeight(fontMetrics.height());
        textRect.setWidth(fontMetrics.width(key));
        textRect.moveCenter(QPoint(pixmapSize.width() / 2, pixmapSize.height() / 2));

        painter.drawText(textRect, Qt::AlignCenter, key);

        painter.end();
        return px;
    //}
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
        case QuietModeToggle:
            return "Audio-QuietMode";
        case TakeScreenshot:
            return "Screen-TakeScreenshot";
        case CaptureScreenVideo:
            return "Screen-CaptureVideo";
        case LockScreen:
            return "System-LockScreen";
        case Run:
            return "System-Run";
        case Suspend:
            return "System-Suspend";
        case PowerOff:
            return "System-PowerOff";
        case NextKeyboardLayout:
            return "Kbd-NextLayout";
        case KeyboardBrightnessUp:
            return "Kbd-BrightnessUp";
        case KeyboardBrightnessDown:
            return "Kbd-BrightnessDown";
        case OpenGateway:
            return "System-OpenGateway";
        case PowerOptions:
            return "System-PowerOptions";
        case Eject:
            return "System-Eject";
    }
}

struct KeyCodeAndModifier {
    KeyCode kc;
    unsigned long modifier;

    bool operator <(const KeyCodeAndModifier& other) const {
        if (other.kc == this->kc) {
            return other.modifier < this->modifier;
        } else {
            return other.kc < this->kc;
        }
    }
};

struct GlobalKeyboardKeyPrivate {
    QKeySequence key;

    QString section;
    QString name;
    QString description;

    bool grabbed = false;

    static QMap<KeyCodeAndModifier, int> grabbedKeycodes;
};

QMap<KeyCodeAndModifier, int> GlobalKeyboardKeyPrivate::grabbedKeycodes;

GlobalKeyboardKey::GlobalKeyboardKey(QKeySequence key, QString section, QString name, QString description, QObject* parent) : QObject(parent) {
    d = new GlobalKeyboardKeyPrivate();
    d->key = key;
    d->section = section;
    d->description = description;
    d->name = name;

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
    unsigned long mod = nativeModifiers(0);
    const KeyCodeAndModifier kcm = {kc, mod};
    if (d->grabbedKeycodes.contains(kcm)) {
        d->grabbedKeycodes.insert(kcm, d->grabbedKeycodes.value(kcm) + 1);
    } else {
        d->grabbedKeycodes.insert(kcm, 1);
        int retval = XGrabKey(QX11Info::display(), kc, mod, QX11Info::appRootWindow(), true, GrabModeAsync, GrabModeAsync);
        //if (retval != 1) {
            qDebug() << "XGrabKey returned" << retval << "for" << d->description << "with keycode" << kc;
        //}
    }
    d->grabbed = true;
}

void GlobalKeyboardKey::ungrabKey() {
    if (!d->grabbed) return;
    //Ungrab this key
    KeyCode kc = XKeysymToKeycode(QX11Info::display(), nativeKey(0));
    unsigned long mod = nativeModifiers(0);
    const KeyCodeAndModifier kcm = {kc, mod};
    if (d->grabbedKeycodes.value(kcm) == 1) {
        XUngrabKey(QX11Info::display(), kc, nativeModifiers(0), QX11Info::appRootWindow());
        d->grabbedKeycodes.remove(kcm);
    } else {
        d->grabbedKeycodes.insert(kcm, d->grabbedKeycodes.value(kcm) - 1);
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

QKeySequence GlobalKeyboardKey::key() {
    return d->key;
}

QString GlobalKeyboardKey::name() {
    return d->name;
}
