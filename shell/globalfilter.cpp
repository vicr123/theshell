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

#include "globalfilter.h"

#include "mainwindow.h"
#include <QThread>
#include <globalkeyboard/globalkeyboardengine.h>

#include "screenshotwindow.h"
#include "dbusevents.h"

extern void playSound(QUrl, bool = false);
extern void EndSession(EndSessionWait::shutdownType type);
extern MainWindow* MainWin;
extern DbusEvents* DBusEvents;

Background* firstBackground;
GlobalFilter::GlobalFilter(QApplication *application, QObject *parent) : QObject(parent)
{
   application->installEventFilter(this);

   connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(reloadScreens()));
   connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(reloadScreens()));
   connect(QApplication::desktop(), SIGNAL(primaryScreenChanged()), this, SLOT(reloadScreens()));
   connect(MainWin, SIGNAL(reloadBackgrounds()), this, SLOT(reloadBackgrounds()));

   connect(GlobalKeyboardEngine::instance(), &GlobalKeyboardEngine::keyShortcutRegistered, this, [=](QString name, GlobalKeyboardKey* key) {
       if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::TakeScreenshot)) {
           connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
               screenshotWindow* screenshot = new screenshotWindow();
               screenshot->show();
           });
       } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::LockScreen)) {
           connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
               DBusEvents->LockScreen();
           });
       } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::Run)) {
           connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
               RunDialog* run = new RunDialog();
               run->show();
           });
       } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::Suspend)) {
           connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
               EndSession(EndSessionWait::suspend);
           });
       } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::PowerOff)) {
           connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
               //Perform an action depending on what the user wants
               switch (settings.value("power/onPowerButtonPressed", 0).toInt()) {
                   case 0: { //Ask what to do
                       EndSessionWait* endSession;
                       if (settings.value("input/touch", false).toBool()) {
                           endSession = new EndSessionWait(EndSessionWait::slideOff);
                       } else {
                           endSession = new EndSessionWait(EndSessionWait::ask);
                       }
                       endSession->showFullScreen();
                       endSession->exec();
                       endSession->deleteLater();
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
           });
       } else if (name == GlobalKeyboardEngine::keyName(GlobalKeyboardEngine::PowerOptions)) {
           connect(key, &GlobalKeyboardKey::shortcutActivated, this, [=] {
               EndSessionWait* endSession;
               if (settings.value("input/touch", false).toBool()) {
                   endSession = new EndSessionWait(EndSessionWait::slideOff);
               } else {
                   endSession = new EndSessionWait(EndSessionWait::ask);
               }
               endSession->showFullScreen();
               endSession->exec();
               endSession->deleteLater();
           });
       }
   });

   reloadScreens();
}

bool GlobalFilter::eventFilter(QObject *object, QEvent *event) {
    Q_UNUSED(object)
    if (event->type() == QEvent::MouseButtonRelease) {
        QSettings settings;
        if (settings.value("input/touchFeedbackSound", false).toBool()) {
            QSound::play(":/sounds/click.wav");
        }
    }
    return false;
}

void GlobalFilter::reloadScreens() {
    QThread::msleep(500); //Wait for KScreen to apply screen changes
    emit removeBackgrounds();
    for (int i = 0; i < QApplication::desktop()->screenCount(); i++) {
        Background* w = new Background(MainWin, i == 0 ? true : false, QApplication::desktop()->screenGeometry(i));
        w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
        w->setAttribute(Qt::WA_ShowWithoutActivating, true);
        w->show();
        w->setGeometry(QApplication::desktop()->screenGeometry(i));
        w->showFullScreen();
        connect(this, SIGNAL(removeBackgrounds()), w, SLOT(deleteLater()));
        connect(this, SIGNAL(changeBackgrounds()), w, SLOT(changeBackground()));
        if (i == 0) {
            firstBackground = w;
            connect(w, SIGNAL(reloadBackground()), this, SLOT(reloadBackgrounds()));
        } else {
            connect(firstBackground, SIGNAL(setAllBackgrounds(QString)), w, SLOT(setCommunityBackground(QString)));
        }
    }
}

void GlobalFilter::reloadBackgrounds() {
    emit changeBackgrounds();
}

void GlobalFilter::newCommunityImage() {

}
