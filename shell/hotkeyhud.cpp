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

#include "hotkeyhud.h"
#include "ui_hotkeyhud.h"

extern float getDPIScaling();

HotkeyHud::HotkeyHud(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HotkeyHud)
{
    ui->setupUi(this);

    this->resize(this->width() * getDPIScaling(), this->height() * getDPIScaling());
}

HotkeyHud::~HotkeyHud()
{
    delete ui;
}

void HotkeyHud::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void HotkeyHud::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void HotkeyHud::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);

    QPalette pal = this->palette();

    QColor highlightCol = pal.color(QPalette::Window);
    int average = (highlightCol.red() + highlightCol.green() + highlightCol.blue()) / 3;
    int value = this->value;
    while (value > 0) {
        if (average < 127) { //Dark color
            highlightCol = highlightCol.light(150);
        } else {
            highlightCol = highlightCol.dark(150);
        }
        painter.setBrush(highlightCol);
        painter.setPen(Qt::transparent);
        painter.drawRect(0, 0, ((float) value / (float) 100) * this->width(), this->height() - 1);
        value -= 100;
    }

    event->accept();
}

void HotkeyHud::show(int timeout) {
    Atom atoms[2];
    atoms[0] = XInternAtom(QX11Info::display(), "_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY", False);
    atoms[1] = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
    int retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &atoms, 2); //Change Window Type

    unsigned long desktop = 0xFFFFFFFF;
    retval = XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    QDialog::show();

    if (!isShowing) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        this->setGeometry(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height());

        tPropertyAnimation *anim = new tPropertyAnimation(this, "geometry");
        anim->setStartValue(this->geometry());
        anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), this->height()));
        anim->setDuration(100);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start();
        connect(anim, &tPropertyAnimation::finished, [=](){
            this->repaint();
        });
    }

    if (this->timeout == NULL) {
        this->timeout = new QTimer();
        this->timeout->setSingleShot(true);
        this->timeout->setInterval(timeout);
        connect(this->timeout, SIGNAL(timeout()), this, SLOT(Timeout()));

    }
    this->timeout->start();

    isShowing = true;
}

void HotkeyHud::show(QIcon icon, QString control, int value) {
    ui->icon->setPixmap(icon.pixmap(32));
    ui->control->setText(control);
    ui->value->setText(QString::number(value) + "%");
    ui->explanation->setText("");
    ui->value->setVisible(true);
    this->value = value;
    this->show();
    this->repaint();
}

void HotkeyHud::show(QIcon icon, QString control, QString explanation, int timeout) {
    ui->icon->setPixmap(icon.pixmap(32));
    ui->control->setText(control);
    ui->explanation->setText(explanation);
    ui->value->setVisible(false);
    ui->explanation->setVisible(true);
    this->value = 0;
    this->show(timeout);
    this->repaint();
}

void HotkeyHud::Timeout() {
    timeout->deleteLater();
    timeout = NULL;
    this->close();
}

void HotkeyHud::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    tPropertyAnimation *anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tPropertyAnimation::finished, [=]() {
        QDialog::close();
        isShowing = false;
    });
    anim->start();
}
