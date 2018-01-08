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

#include "globalfilter.h"

extern void playSound(QUrl, bool = false);
extern MainWindow* MainWin;
GlobalFilter::GlobalFilter(QApplication *application, QObject *parent) : QObject(parent)
{
   application->installEventFilter(this);

   connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(reloadScreens()));
   connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(reloadScreens()));
   connect(QApplication::desktop(), SIGNAL(primaryScreenChanged()), this, SLOT(reloadScreens()));
   connect(MainWin, SIGNAL(reloadBackgrounds()), this, SLOT(reloadBackgrounds()));

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
    reloadBackgrounds();
}

void GlobalFilter::reloadBackgrounds() {
    emit removeBackgrounds();
    for (int i = 0; i < QApplication::desktop()->screenCount(); i++) {
        Background* w = new Background(MainWin, QApplication::desktop()->screenGeometry(i));
        w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
        w->setAttribute(Qt::WA_ShowWithoutActivating, true);
        w->show();
        w->setGeometry(QApplication::desktop()->screenGeometry(i));
        w->showFullScreen();
        connect(this, SIGNAL(removeBackgrounds()), w, SLOT(deleteLater()));
    }
}
