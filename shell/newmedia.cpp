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

#include "newmedia.h"
#include "ui_newmedia.h"

#define False 0
#define True 1

NewMedia::NewMedia(QString description, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::NewMedia) {
    ui->setupUi(this);

    ui->description->setText(description);
    if (!QFile("/usr/bin/thefile").exists()) {
        ui->openFiles->setVisible(false);
    }
    ui->openFiles->setVisible(false);
}

NewMedia::~NewMedia() {
    delete ui;
}


void NewMedia::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
        QString::number(x) + "," + QString::number(y) + "," +
        QString::number(w) + "," + QString::number(h));
}

void NewMedia::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void NewMedia::show() {
    Atom DesktopWindowTypeAtom;
    DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
    int retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
            XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type

    unsigned long desktop = 0xFFFFFFFF;
    retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
            XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    QDialog::show();

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    this->setGeometry(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height());

    tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), this->height()));
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tPropertyAnimation::finished, [ = ]() {
        this->repaint();
    });
    anim->start();

    XEvent event;

    event.xclient.type = ClientMessage;
    event.xclient.serial = 0;
    event.xclient.send_event = True;
    event.xclient.message_type = XInternAtom(QX11Info::display(), "_NET_ACTIVE_WINDOW", False);
    event.xclient.window = this->winId();
    event.xclient.format = 32;
    event.xclient.data.l[0] = 2;

    XSendEvent(QX11Info::display(), DefaultRootWindow(QX11Info::display()), False, SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

void NewMedia::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &tPropertyAnimation::finished, [ = ]() {
        QDialog::close();
    });
    anim->start();
}

void NewMedia::reject() {
    this->close();
}

void NewMedia::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
    event->accept();
}

void NewMedia::on_closeButton_clicked() {
    this->close();
}

void NewMedia::on_doNothingOptionToolButton_clicked() {
    this->close();
}
