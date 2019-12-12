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
#include "userspane.h"
#include "ui_userspane.h"

#include "usersmodel.h"
#include "user.h"
#include <PolkitQt1/Authority>
#include <tpopover.h>
#include <ttoast.h>
#include "adduserdialog.h"
#include "deleteuserdialog.h"

struct UsersPanePrivate {
    UserPtr currentUser;
};

UsersPane::UsersPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UsersPane)
{
    ui->setupUi(this);
    d = new UsersPanePrivate();

    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-user");
    this->settingAttributes.menuWidget = ui->leftPane;
    this->settingAttributes.providesLeftPane = true;

    ui->usersList->setModel(new UsersModel());
    connect(ui->usersList->selectionModel(), &QItemSelectionModel::currentChanged, this, [=](QModelIndex newIndex, QModelIndex oldIndex) {
        if (d->currentUser) {
            disconnect(d->currentUser.data(), &User::dataUpdated, this, &UsersPane::currentUserChanged);
        }

        if (newIndex.isValid()) {
            ui->stackedWidget->setCurrentWidget(ui->userPane);

            d->currentUser = newIndex.data(Qt::UserRole).value<UserPtr>();
            connect(d->currentUser.data(), &User::dataUpdated, this, &UsersPane::currentUserChanged);
            this->currentUserChanged();
        } else {
            d->currentUser = nullptr;
            ui->stackedWidget->setCurrentWidget(ui->noUserPane);
        }
    });

    ui->deleteUserButton->setProperty("type", "destructive");
}

UsersPane::~UsersPane()
{
    delete d;
    delete ui;
}

QWidget*UsersPane::mainWidget()
{
    return this;
}

QString UsersPane::name()
{
    return tr("Users");
}

StatusCenterPaneObject::StatusPaneTypes UsersPane::type()
{
    return Setting;
}

int UsersPane::position()
{
    return 800;
}

void UsersPane::message(QString name, QVariantList args)
{

}

void UsersPane::on_mainMenuButton_clicked()
{
    sendMessage("main-menu", QVariantList());
}

void UsersPane::currentUserChanged()
{
    ui->usernameTitleLabel->setText(d->currentUser->displayName());
    ui->usernameLabel->setText(tr("Username: %1").arg(d->currentUser->userName()));

    if (d->currentUser->isLocked()) {
        ui->lockUserButton->setText(tr("Unlock User"));
        ui->lockUserButton->setDescription(tr("Allow this user to log in"));
    } else {
        ui->lockUserButton->setText(tr("Lock User"));
        ui->lockUserButton->setDescription(tr("Stop this user from logging in"));
    }
}

void UsersPane::on_addButton_clicked()
{
    this->checkPolkit(false)->then([=] {
        AddUserDialog* d = new AddUserDialog();
        tPopover* popover = new tPopover(d);
        popover->setPopoverWidth(SC_DPI(600));
        popover->setDismissable(false);
        connect(d, &AddUserDialog::done, popover, &tPopover::dismiss);
        connect(popover, &tPopover::dismissed, d, &AddUserDialog::deleteLater);
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        popover->show(this->window());
    });
}

tPromise<void>*UsersPane::checkPolkit(bool isOwnUser)
{
    tPromise<void>* promise = new tPromise<void>([=](QString& error) {
        QString auth;
        if (isOwnUser) {
            auth = "org.freedesktop.accounts.change-own-user-data";
        } else {
            auth = "org.freedesktop.accounts.user-administration";
        }

        //Check Polkit authorization
        PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync(auth, PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::None);
        if (r == PolkitQt1::Authority::No) {
            error = "not-allowed";
            return;
        } else if (r == PolkitQt1::Authority::Challenge) {
    //        LOWER_INFOPANE
            PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync(auth, PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::AllowUserInteraction);
            if (r != PolkitQt1::Authority::Yes) {
                error = "challenge-failed";
                return;
            }
        }
    });
    promise->error([=](QString err) {
        if (err == "not-allowed") {
            tToast* toast = new tToast();
            toast->setTitle(tr("Unauthorized"));
            toast->setText(tr("Polkit does not allow you to manage users on the system."));
            connect(toast, &tToast::dismissed, toast, &tToast::deleteLater);
            toast->show(this);
        }
    });
    return promise;
}

void UsersPane::on_deleteUserButton_clicked()
{
    this->checkPolkit(false)->then([=] {
        DeleteUserDialog* d = new DeleteUserDialog(this->d->currentUser);
        tPopover* popover = new tPopover(d);
        popover->setPopoverWidth(SC_DPI(600));
        popover->setDismissable(false);
        connect(d, &DeleteUserDialog::done, popover, &tPopover::dismiss);
        connect(popover, &tPopover::dismissed, d, &AddUserDialog::deleteLater);
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        popover->show(this->window());
    });
}
