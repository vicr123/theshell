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
#ifndef GLOBALKEYBOARDENGINE_H
#define GLOBALKEYBOARDENGINE_H

#include <QObject>
#include <QAbstractNativeEventFilter>

struct GlobalKeyboardKeyPrivate;
class GlobalKeyboardKey : public QObject
{
        Q_OBJECT
    public:
        explicit GlobalKeyboardKey(QKeySequence key, QString section = "", QString description = "", QObject* parent = nullptr);
        ~GlobalKeyboardKey();

        int chordCount();
        unsigned long nativeKey(uint chordNumber);
        unsigned long nativeModifiers(uint chordNumber);
        QString section();
        QString description();

        void deregister();

        void grabKey();
        void ungrabKey();

    signals:
        void shortcutActivated();
        void deregistered();

    private:
        GlobalKeyboardKeyPrivate* d;
};

struct GlobalKeyboardEnginePrivate;
class GlobalKeyboardEngine : public QObject, public QAbstractNativeEventFilter
{
        Q_OBJECT
    public:
        static GlobalKeyboardKey* registerKey(QKeySequence keySequence, QString name);

        static GlobalKeyboardEngine* instance();

        enum KnownKeyNames {
            BrightnessUp,
            BrightnessDown,
            VolumeUp,
            VolumeDown
        };
        static QString keyName(KnownKeyNames name);

        static void pauseListening();
        static void startListening();

    signals:
        void keyShortcutRegistered(QString name, GlobalKeyboardKey* shortcut);

    public slots:

    private:
        explicit GlobalKeyboardEngine(QObject *parent = nullptr);
        static GlobalKeyboardEnginePrivate* d;

        bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};


#endif // GLOBALKEYBOARDENGINE_H
