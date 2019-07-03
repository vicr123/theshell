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
#include "simpinrequest.h"
#include "ui_simpinrequest.h"

#include "../enums.h"
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusPendingCallWatcher>
#include <terrorflash.h>

#include <modem.h>
#include <sim.h>

struct SimPinRequestPrivate {
    QString modemPath;

    ModemManager::Modem* modem;
    ModemManager::Sim* sim;

    MMModemLock lockType;
};

SimPinRequest::SimPinRequest(QString modemPath, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimPinRequest)
{
    ui->setupUi(this);
    d = new SimPinRequestPrivate();

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::Fade);

    d->modemPath = modemPath;
    d->modem = new ModemManager::Modem(modemPath);
    d->sim = new ModemManager::Sim(d->modem->simPath());

    checkSimLock();
}

SimPinRequest::~SimPinRequest()
{
    d->sim->deleteLater();
    d->modem->deleteLater();
    delete ui;
    delete d;
}

void SimPinRequest::on_backButton_clicked()
{
    emit done();
}

void SimPinRequest::on_okButton_clicked()
{

    switch (d->lockType) {
        case MM_MODEM_LOCK_SIM_PIN:
        case MM_MODEM_LOCK_SIM_PIN2:
            if (!QRegularExpression("^\\d{4,8}$").match(ui->pinField->text()).hasMatch()) {
                tErrorFlash::flashError(ui->pinField);
            } else {
                ui->stackedWidget->setCurrentIndex(1);
                QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(d->sim->sendPin(ui->pinField->text()));
                connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
                    watcher->deleteLater();
                    QTimer::singleShot(500, this, &SimPinRequest::checkSimLock);
                });
            }
            break;
        case MM_MODEM_LOCK_SIM_PUK:
        case MM_MODEM_LOCK_SIM_PUK2:
            if (!QRegularExpression("^\\d{0,8}$").match(ui->pinField->text()).hasMatch()) {
                tErrorFlash::flashError(ui->pinField);
            } else if (!QRegularExpression("^\\d{4,8}$").match(ui->newPinField->text()).hasMatch()) {
                tErrorFlash::flashError(ui->newPinField);
            } else if (ui->newPinField->text() != ui->confirmNewPinField->text()) {
                tErrorFlash::flashError(ui->confirmNewPinField);
            } else {
                ui->stackedWidget->setCurrentIndex(1);

                //PUK field uses the same field as the PIN field
                QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(d->sim->sendPuk(ui->pinField->text(), ui->newPinField->text()));
                connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
                    watcher->deleteLater();
                    QTimer::singleShot(500, this, &SimPinRequest::checkSimLock);
                });
            }
            break;
        default: ;
    }
}

void SimPinRequest::checkSimLock() {
    ui->stackedWidget->setCurrentIndex(0);
    ui->pinField->setText("");
    ui->newPinField->setText("");
    ui->confirmNewPinField->setText("");

    //Ensure we haven't locked ourselves out
    if (d->modem->stateFailedReason() == MM_MODEM_STATE_FAILED_REASON_SIM_ERROR) {
        //We've probably locked ourselves out by getting the PUK incorrect too many times
        //Bail out before we do any more damage ;)
        emit done();
        return;
    }

    //Find out why the device is locked
    d->lockType = d->modem->unlockRequired();

    QString lockTypeString;
    switch (d->lockType) {
        case MM_MODEM_LOCK_NONE:
            //We're done here!
            emit done();
            return;
        case MM_MODEM_LOCK_SIM_PIN:
            lockTypeString = tr("SIM PIN");
            ui->newPinRequest->setVisible(false);
            ui->newPinField->setVisible(false);
            ui->pukInfoLabel->setVisible(false);
            ui->confirmNewPinField->setVisible(false);
            break;
        case MM_MODEM_LOCK_SIM_PUK:
            lockTypeString = tr("SIM PUK");
            ui->newPinRequest->setVisible(true);
            ui->newPinField->setVisible(true);
            ui->newPinRequest->setText(tr("Additionally, provide a new <b>%1</b> to set after the SIM is unlocked.").arg(tr("SIM PIN")));
            ui->pukInfoLabel->setVisible(true);
            ui->confirmNewPinField->setVisible(true);
            break;
        case MM_MODEM_LOCK_SIM_PIN2:
            lockTypeString = tr("SIM PIN2");
            ui->newPinRequest->setVisible(false);
            ui->newPinField->setVisible(false);
            ui->pukInfoLabel->setVisible(false);
            ui->confirmNewPinField->setVisible(false);
            break;
        case MM_MODEM_LOCK_SIM_PUK2:
            lockTypeString = tr("SIM PUK2");
            ui->newPinRequest->setVisible(true);
            ui->newPinField->setVisible(true);
            ui->newPinRequest->setText(tr("Additionally, provide a new <b>%1</b> to set after the SIM is unlocked.").arg(tr("SIM PIN2")));
            ui->pukInfoLabel->setVisible(true);
            ui->confirmNewPinField->setVisible(true);
            break;
        default: ;
    }

    ui->pinField->setPlaceholderText(lockTypeString);

    QDBusMessage retriesMessage = QDBusMessage::createMethodCall("org.freedesktop.ModemManager1", d->modemPath, "org.freedesktop.DBus.Properties", "Get");
    retriesMessage.setArguments({"org.freedesktop.ModemManager1.Modem", "UnlockRetries"});
    QDBusMessage retriesReplyMessage = QDBusConnection::systemBus().call(retriesMessage);

    QDBusArgument retriesArg = retriesReplyMessage.arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>();
    QMap<uint, uint> unlockRetries;

    retriesArg >> unlockRetries;

    QString requestLabelText = tr("Provide the <b>%1</b> to unlock this SIM card.").arg(lockTypeString);

    if (unlockRetries.contains(d->lockType)) {
        int retryAttempts = static_cast<int>(unlockRetries.value(d->lockType));
        if (retryAttempts == 0) {
            //We've probably locked ourselves out by getting the PUK incorrect too many times
            //Bail out before we do any more damage ;)
            emit done();
            return;
        }
        requestLabelText.append(" " + tr("You have %n attempts remaining.", nullptr, retryAttempts));
    }

    ui->requestLabel->setText(requestLabelText);
    ui->titleLabel->setText(lockTypeString);
    ui->titleLabel_2->setText(lockTypeString);
    ui->stackedWidget->setCurrentIndex(0);
}
