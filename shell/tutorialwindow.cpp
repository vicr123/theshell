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

#include "tutorialwindow.h"
#include "ui_tutorialwindow.h"

extern void sendMessageToRootWindow(const char* message, Window window, long data0 = 0, long data1 = 0, long data2 = 0, long data3 = 0, long data4 = 0);

TutorialWindow::TutorialWindow(bool doSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TutorialWindow)
{
    ui->setupUi(this);

    //this->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
    this->setFocusPolicy(Qt::NoFocus);
    doMask();

    settings.beginGroup("tutorial");

    if (QX11Info::isPlatformX11()) {
        Atom DesktopWindowTypeAtom;
        DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_UTILITY", False);
        XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                         XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type

        unsigned long desktop = 0xFFFFFFFF;
        qDebug() << XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                         XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops
    }

    this->showFullScreen();
    this->doSettings = doSettings;
}

TutorialWindow::~TutorialWindow()
{
    delete ui;
}

void TutorialWindow::doMask() {
    if (maskWidget != NULL) {
        this->setMask(QRegion(maskWidget->x(), maskWidget->y(), maskWidget->sizeHint().width(), maskWidget->sizeHint().height()));
    } else {
        this->setMask(QRegion(-1, -1, 1, 1));
    }
}

void TutorialWindow::resizeEvent(QResizeEvent *event) {
    doMask();
}

void TutorialWindow::showScreen(AvailableScreens screen) {
    switch (screen) {
    case Gateway:
        if (!settings.value("gateway", false).toBool() || doSettings) {
            ui->stack->setCurrentWidget(ui->gatewayPage);
            maskWidget = ui->gatewayFrame;
            settings.setValue("gateway", true);
        }
        break;
    case BarLocation:
        if (!settings.value("barLocation", false).toBool() || doSettings) {
            ui->stack->setCurrentWidget(ui->barLocationPage);
            maskWidget = ui->barLocationFrame;
            settings.setValue("barLocation", true);
        }
        break;
    case GatewaySearch:
        if (!settings.value("gatewaySearch", false).toBool() || doSettings) {
            ui->stack->setCurrentWidget(ui->gatewaySearchPage);
            maskWidget = ui->gatewaySearchFrame;
            settings.setValue("gatewaySearch", true);
        }
        break;
    case MissedNotification:
        if (!settings.value("missedNotification", false).toBool() || doSettings) {
            ui->stack->setCurrentWidget(ui->missedNotificationPage);
            maskWidget = ui->missedNotificationFrame;
            settings.setValue("missedNotification", true);
        }
        break;
    }
    doMask();
}

void TutorialWindow::hideScreen(AvailableScreens screen) {
    bool doSwitch = false;
    switch (screen) {
    case Gateway:
        if (maskWidget == ui->gatewayFrame) doSwitch = true;
        break;
    case BarLocation:
        if (maskWidget == ui->barLocationFrame) doSwitch = true;
        break;
    case GatewaySearch:
        if (maskWidget == ui->gatewaySearchFrame) doSwitch = true;
        break;
    case MissedNotification:
        if (maskWidget == ui->missedNotificationFrame) doSwitch = true;
        break;
    }

    if (doSwitch) maskWidget = NULL;
    doMask();
}

void TutorialWindow::on_dismissMissedNotification_clicked()
{
    hideScreen(MissedNotification);
}
