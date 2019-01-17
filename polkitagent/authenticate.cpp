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

#include "authenticate.h"
#include "ui_authenticate.h"

Authenticate::Authenticate(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Authenticate)
{
    ui->setupUi(this);

    fadeEffect = new QGraphicsOpacityEffect(this);
    ui->authFrame->setGraphicsEffect(fadeEffect);

    connect(tVirtualKeyboard::instance(), &tVirtualKeyboard::keyboardVisibleChanged, [=](bool visible) {
        if (visible) {
            QRect desktopRect = QApplication::desktop()->screenGeometry();
            desktopRect.setHeight(desktopRect.height() - tVirtualKeyboard::instance()->height());
            this->setGeometry(desktopRect);
        } else {
            QRect desktopRect = QApplication::desktop()->screenGeometry();
            this->setGeometry(desktopRect);
        }
    });
}

Authenticate::~Authenticate()
{
    delete ui;
}

void Authenticate::showFullScreen(bool showError) {
    XGrabKeyboard(QX11Info::display(), RootWindow(QX11Info::display(), 0), True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(QX11Info::display(), RootWindow(QX11Info::display(), 0), True, 0, GrabModeAsync, GrabModeAsync, RootWindow(QX11Info::display(), 0), 0, CurrentTime);

    if (showError) {
        tToast* toast = new tToast();
        toast->setTitle(tr("Incorrect Password"));
        toast->setText(tr("The password you entered was incorrect. Please enter your password again."));
        connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
        toast->show(this);
    }
    ui->lineEdit->setText("");

    ui->authFrame->setVisible(true);
    QPropertyAnimation *a = new QPropertyAnimation(fadeEffect, "opacity");
    a->setDuration(250);
    a->setStartValue(0);
    a->setEndValue(1);
    a->start();
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));

    //QDialog::showFullScreen();
    /*Atom DesktopWindowTypeAtom;
    DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NORMAL", False);
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type*/

    unsigned long desktop = 0xFFFFFFFF;
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->show();

    //Show onscreen keyboard if it is running
    tVirtualKeyboard::instance()->showKeyboard();

    ui->keyboardButton->setVisible(tVirtualKeyboard::instance()->isKeyboardRunning());

    QRect desktopRect = QApplication::desktop()->screenGeometry();
    desktopRect.setHeight(desktopRect.height() - tVirtualKeyboard::instance()->height());

    ui->lineEdit->setFocus();

    QTimer::singleShot(0, [=] {
        this->setGeometry(desktopRect);
        this->raise();
    });
}

void Authenticate::setMessage(QString message) {
    ui->message->setText(message);
}

void Authenticate::setIcon(QIcon icon) {
    ui->polkitIcon->setPixmap(icon.pixmap(22, 22));
}

void Authenticate::on_pushButton_2_clicked()
{
    QPropertyAnimation *a = new QPropertyAnimation(fadeEffect, "opacity");
    a->setDuration(250);
    a->setStartValue(1);
    a->setEndValue(0);
    a->start();
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
    connect(a, &QPropertyAnimation::finished, [=]() {
        ui->authFrame->setVisible(false);
        ui->lineEdit->setText("");
    });

    emit okClicked();
}

void Authenticate::setUser(QString user) {
    //ui->authenticationUser->setText(user);

    for (int i = 0; i < ui->authenticationUsers->count(); i++) {
        if (ui->authenticationUsers->item(i)->text() == user) {
            ui->authenticationUsers->setCurrentIndex(ui->authenticationUsers->model()->index(i, 0));
            ui->userLabel->setText(tr("Authenticating as %1").arg(user));
            break;
        }
    }
}

void Authenticate::setUser(PolkitQt1::Identity user) {
    for (int i = 0; i < ui->authenticationUsers->count(); i++) {
        if (ui->authenticationUsers->item(i)->data(Qt::UserRole).value<PolkitQt1::Identity>().toString() == user.toString()) {
            ui->authenticationUsers->setCurrentIndex(ui->authenticationUsers->model()->index(i, 0));
            ui->userLabel->setText(tr("Authenticating as %1").arg(user.toString().remove("unix-user:")));
            break;
        }
    }
}

void Authenticate::setUsers(PolkitQt1::Identity::List users) {
    ui->authenticationUsers->clear();
    for (PolkitQt1::Identity identity : users) {
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(identity.toString().remove("unix-user:"));
        item->setData(Qt::UserRole, QVariant::fromValue(identity));
        ui->authenticationUsers->addItem(item);
        //ui->authenticationUsers->addItem(identity.toString().remove("unix-user:"), QVariant::fromValue(identity));
    }
}

void Authenticate::on_pushButton_clicked()
{
    this->reject();
}

QString Authenticate::getPassword() {
    QString ret = ui->lineEdit->text();
    return ret;
}

void Authenticate::on_lineEdit_returnPressed()
{
    ui->pushButton_2->click();
}


void Authenticate::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void Authenticate::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void Authenticate::on_keyboardButton_clicked()
{
    tVirtualKeyboard::instance()->showKeyboard();
}

void Authenticate::reject() {
    QDialog::reject();
    XUngrabKeyboard(QX11Info::display(), CurrentTime);
    XUngrabPointer(QX11Info::display(), CurrentTime);
}

void Authenticate::setDetails(const PolkitQt1::Details& details) {
    qDebug() << details.keys();
}

void Authenticate::on_backToMain_clicked()
{
    ui->mainStack->setCurrentIndex(0);
}

void Authenticate::on_switchUser_clicked()
{
    ui->mainStack->setCurrentIndex(1);
}

void Authenticate::on_authenticationUsers_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current != nullptr) {
        emit this->newUser(current->data(Qt::UserRole).value<PolkitQt1::Identity>());
        ui->mainStack->setCurrentIndex(0);
    }
}
