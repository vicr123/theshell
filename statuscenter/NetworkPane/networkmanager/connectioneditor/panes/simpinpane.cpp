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
#include "simpinpane.h"
#include "ui_simpinpane.h"

#include <QRegularExpression>
#include <terrorflash.h>
#include <sim.h>
#include <ttoast.h>

struct SimPinPanePrivate {
    ModemManager::Modem::Ptr modem;
    ModemManager::Sim::Ptr sim;

    enum CurrentAction {
        EnablePin,
        DisablePin,
        ChangePin
    };

    CurrentAction currentAction;
};

SimPinPane::SimPinPane(ModemManager::Modem::Ptr modem, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SimPinPane)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);

    d = new SimPinPanePrivate();
    d->modem = modem;
    d->sim.reset(new ModemManager::Sim(d->modem->simPath()));

    reloadSimStatus();
}

SimPinPane::~SimPinPane()
{
    delete ui;
    delete d;
}

void SimPinPane::on_backButton_clicked()
{
    if (d->currentAction == SimPinPanePrivate::ChangePin) {
        ui->stackedWidget->setCurrentIndex(1);
    } else {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void SimPinPane::on_enablePinButton_clicked()
{
    d->currentAction = SimPinPanePrivate::EnablePin;
    ui->enablePinTitle->setText(tr("Enable SIM PIN"));
    ui->stackedWidget->setCurrentIndex(2);
}

void SimPinPane::on_disablePinButton_clicked()
{
    d->currentAction = SimPinPanePrivate::DisablePin;
    ui->enablePinTitle->setText(tr("Disable SIM PIN"));
    ui->stackedWidget->setCurrentIndex(2);
}

void SimPinPane::on_enablePinButtonGo_clicked()
{
    this->setEnabled(false);
    if (!QRegularExpression("^\\d{4,8}$").match(ui->enablePinField->text()).hasMatch()) {
        tErrorFlash::flashError(ui->enablePinFieldWrapper);
        return;
    }

    if (d->currentAction == SimPinPanePrivate::ChangePin) {
        QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(d->sim->changePin(ui->enablePinField->text(), ui->changePinField->text()));
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
            tToast* toast = new tToast(this);

            this->setEnabled(true);
            if (watcher->reply().type() == QDBusMessage::ErrorMessage) {
                if (watcher->reply().errorName() == "org.freedesktop.ModemManager1.Error.MobileEquipment.IncorrectPassword" ||
                        watcher->reply().errorName() == "org.freedesktop.ModemManager1.Error.MobileEquipment.SimPuk") {
                    toast->setTitle(tr("Incorrect SIM PIN"));
                    toast->setText(tr("The incorrect SIM PIN was given."));
                } else {
                    toast->setTitle(tr("That didn't work"));
                    toast->setText(tr("There was a problem trying to change the SIM PIN."));
                }
            } else {
                toast->setTitle(tr("SIM PIN changed"));
                toast->setText(tr("The SIM PIN has been changed successfully."));
                ui->stackedWidget->setCurrentIndex(0);
            }
            toast->show(this);
            connect(toast, &tToast::dismiss, toast, &tToast::deleteLater);
            watcher->deleteLater();

            ui->enablePinField->setText("");
            reloadSimStatus();
        });
    } else {
        QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(d->sim->enablePin(ui->enablePinField->text(), d->currentAction == SimPinPanePrivate::EnablePin));
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] {
            tToast* toast = new tToast(this);

            this->setEnabled(true);
            if (watcher->reply().type() == QDBusMessage::ErrorMessage) {
                if (watcher->reply().errorName() == "org.freedesktop.ModemManager1.Error.MobileEquipment.IncorrectPassword" ||
                        watcher->reply().errorName() == "org.freedesktop.ModemManager1.Error.MobileEquipment.SimPuk") {
                    toast->setTitle(tr("Incorrect SIM PIN"));
                    toast->setText(tr("The incorrect SIM PIN was given."));
                } else {
                    toast->setTitle(tr("That didn't work"));
                    toast->setText(d->currentAction == SimPinPanePrivate::EnablePin ? tr("There was a problem trying to enable the SIM PIN.") : tr("There was a problem trying to disable the SIM PIN."));
                }
            } else {
                toast->setTitle(d->currentAction == SimPinPanePrivate::EnablePin ? tr("SIM PIN Enabled") : tr("SIM PIN Disabled"));
                toast->setText(d->currentAction == SimPinPanePrivate::EnablePin ? tr("The SIM PIN has been enabled successfully.") : tr("The SIM PIN has been disabled successfully."));
                ui->stackedWidget->setCurrentIndex(0);
            }
            toast->show(this);
            connect(toast, &tToast::dismiss, toast, &tToast::deleteLater);
            watcher->deleteLater();

            ui->enablePinField->setText("");
            reloadSimStatus();
        });
    }
}

void SimPinPane::reloadSimStatus() {
    ui->enableOperatorName->setText(d->sim->operatorName());
    ui->changeOperatorName->setText(d->sim->operatorName());

    //Find out if the device is locked
    if (d->modem->unlockRequired() != MM_MODEM_LOCK_NONE) {
        //Can't do anything now, the SIM is locked
        ui->enablePinButton->setEnabled(false);
        ui->disablePinButton->setEnabled(false);
        ui->changePinButton->setEnabled(false);

        ui->stackedWidget->setCurrentIndex(0);
    } else {
        ui->enablePinButton->setEnabled(true);
        ui->disablePinButton->setEnabled(true);
        ui->changePinButton->setEnabled(true);

        //Get remaining number of attempts until we're kicked out
        QDBusMessage retriesMessage = QDBusMessage::createMethodCall("org.freedesktop.ModemManager1", d->modem->uni(), "org.freedesktop.DBus.Properties", "Get");
        retriesMessage.setArguments({"org.freedesktop.ModemManager1.Modem", "UnlockRetries"});
        QDBusMessage retriesReplyMessage = QDBusConnection::systemBus().call(retriesMessage);

        QDBusArgument retriesArg = retriesReplyMessage.arguments().first().value<QDBusVariant>().variant().value<QDBusArgument>();
        QMap<uint, uint> unlockRetries;

        retriesArg >> unlockRetries;

        int attemptsRemaining = static_cast<int>(unlockRetries.value(MM_MODEM_LOCK_SIM_PIN));
        if (attemptsRemaining == 0) {
            //Can't do anything any more, let's bail!
            ui->enablePinButton->setEnabled(false);
            ui->disablePinButton->setEnabled(false);
            ui->changePinButton->setEnabled(false);

            ui->stackedWidget->setCurrentIndex(0);
        } else {
            QString attemptsText = tr("You have %n attempts remaining", nullptr, attemptsRemaining);
            ui->enablePinAttemptsRemaining->setText(attemptsText);
        }
    }
}

void SimPinPane::on_stackedWidget_currentChanged(int arg1)
{
    if (arg1 == 0) {
        ui->enablePinField->setText("");
        ui->changePinField->setText("");
    } else if (arg1 == 1) {
        ui->changePinField->setFocus();
    } else if (arg1 == 2) {
        ui->enablePinField->setFocus();
    }
}

void SimPinPane::on_backButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void SimPinPane::on_changePinButtonGo_clicked()
{
    if (!QRegularExpression("^\\d{4,8}$").match(ui->changePinField->text()).hasMatch()) {
        tErrorFlash::flashError(ui->changePinFieldWrapper);
        return;
    }

    ui->stackedWidget->setCurrentIndex(2);
}

void SimPinPane::on_changePinButton_clicked()
{
    d->currentAction = SimPinPanePrivate::ChangePin;
    ui->enablePinTitle->setText(tr("Change SIM PIN"));
    ui->stackedWidget->setCurrentIndex(1);
}

void SimPinPane::on_changePinField_returnPressed()
{
    ui->changePinButtonGo->click();
}

void SimPinPane::on_enablePinField_returnPressed()
{
    ui->enablePinButtonGo->click();
}
