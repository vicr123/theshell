/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef TUTORIALWINDOW_H
#define TUTORIALWINDOW_H

#include <QDialog>
#include <QRegion>
#include <QFrame>
#include <QSettings>
#include <QStackedWidget>
#include <QX11Info>
#include <QDebug>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Expose
#undef Status

namespace Ui {
class TutorialWindow;
}

class TutorialWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TutorialWindow(bool doSettings, QWidget *parent = 0);
    ~TutorialWindow();

    enum AvailableScreens {
        Gateway,
        BarLocation,
        GatewaySearch,
        MissedNotification
    };

public slots:
    void showScreen(AvailableScreens screen);
    void hideScreen(AvailableScreens screen);

private slots:
    void on_dismissMissedNotification_clicked();

private:
    Ui::TutorialWindow *ui;
    QWidget* maskWidget = NULL;
    QSettings settings;

    bool doSettings;

    void doMask();

    void resizeEvent(QResizeEvent* event);
};

#endif // TUTORIALWINDOW_H
