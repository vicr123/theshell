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

#include "reminderspage.h"
#include "ui_reminderspage.h"

RemindersPage::RemindersPage(QWidget *parent) :
    QStackedWidget(parent),
    ui(new Ui::RemindersPage)
{
    ui->setupUi(this);

    model = new RemindersListModel();
    ui->remindersList->setModel(model);
    ui->remindersList->setItemDelegate(new RemindersDelegate);

    QTimer* reminderChecker = new QTimer();
    reminderChecker->setInterval(1000);
    connect(reminderChecker, SIGNAL(timeout()), this, SLOT(checkReminders()));
    reminderChecker->start();

    notificationInterface = new QDBusInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
}

RemindersPage::~RemindersPage()
{
    delete ui;
}

void RemindersPage::on_backButton_clicked()
{
    this->setCurrentIndex(0);
}

void RemindersPage::on_addButton_clicked()
{
    ui->reminderTitle->setText("");
    ui->reminderDate->setDateTime(QDateTime::currentDateTime().addSecs(3600)); //Current date + 1 hour

    this->setCurrentIndex(1);
}

void RemindersPage::updateReminders() {
    model->updateData();
}

void RemindersPage::on_addReminderButton_clicked()
{
    if (ui->reminderTitle->text() == "") {
        tToast* toast = new tToast();
        toast->setTitle(tr("Title required"));
        toast->setText("You'll need to enter a title for this reminder.");
        connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
        toast->show(this);
        return;
    }

    QList<QPair<QString, QDateTime>> ReminderData;

    QSettings reminders("theSuite/theShell.reminders");
    reminders.beginGroup("reminders");
    int count = reminders.beginReadArray("reminders");

    for (int i = 0; i < count; i++) {
        reminders.setArrayIndex(i);
        QPair<QString, QDateTime> data;
        data.first = reminders.value("title").toString();
        data.second = reminders.value("date").toDateTime();
        ReminderData.append(data);
    }

    QPair<QString, QDateTime> newData;
    newData.first = ui->reminderTitle->text();
    newData.second = ui->reminderDate->dateTime().addSecs(-ui->reminderDate->dateTime().time().second());
    ReminderData.append(newData);

    reminders.endArray();
    reminders.beginWriteArray("reminders");
    int i = 0;
    for (QPair<QString, QDateTime> data : ReminderData) {
        reminders.setArrayIndex(i);
        reminders.setValue("title", data.first);
        reminders.setValue("date", data.second);
        i++;
    }
    reminders.endArray();
    reminders.endGroup();

    model->updateData();
    this->setCurrentIndex(0);
}

void RemindersPage::on_deleteButton_clicked()
{
    if (ui->remindersList->selectionModel()->selectedIndexes().count() == 0) return;

    QList<QPair<QString, QDateTime>> ReminderData;

    QSettings reminders("theSuite/theShell.reminders");
    reminders.beginGroup("reminders");
    int count = reminders.beginReadArray("reminders");

    for (int i = 0; i < count; i++) {
        reminders.setArrayIndex(i);
        QPair<QString, QDateTime> data;
        data.first = reminders.value("title").toString();
        data.second = reminders.value("date").toDateTime();
        ReminderData.append(data);
    }

    reminders.endArray();

    int row = ui->remindersList->selectionModel()->selectedIndexes().at(0).row();
    QPair<QString, QDateTime> data = ReminderData.takeAt(row);

    reminders.beginWriteArray("reminders");
    int i = 0;
    for (QPair<QString, QDateTime> data : ReminderData) {
        reminders.setArrayIndex(i);
        reminders.setValue("title", data.first);
        reminders.setValue("date", data.second);
        i++;
    }
    reminders.endArray();
    reminders.endGroup();

    model->updateData();

    QMap<QString, QString> actions;
    actions.insert("undo", tr("Undo"));

    tToast* toast = new tToast();
    toast->setTitle(tr("Reminder Deleted"));
    toast->setText(tr("%1 has been deleted.").arg(data.first));
    toast->setActions(actions);
    connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
    connect(toast, &tToast::actionClicked, [=](QString key) {
        if (key == "undo") {
            //Undo the removal
            QList<QPair<QString, QDateTime>> dat = ReminderData;
            dat.insert(row, data);

            QSettings reminders("theSuite/theShell.reminders");
            reminders.beginGroup("reminders");
            reminders.beginWriteArray("reminders");
            int i = 0;
            for (QPair<QString, QDateTime> data : dat) {
                reminders.setArrayIndex(i);
                reminders.setValue("title", data.first);
                reminders.setValue("date", data.second);
                i++;
            }
            reminders.endArray();
            reminders.endGroup();

            model->updateData();
        }
    });
    toast->show(this);
}

void RemindersPage::checkReminders() {
    QList<QPair<QString, QDateTime>> ReminderData;

    QSettings reminders("theSuite/theShell.reminders");
    reminders.beginGroup("reminders");
    int count = reminders.beginReadArray("reminders");

    for (int i = 0; i < count; i++) {
        reminders.setArrayIndex(i);
        QPair<QString, QDateTime> data;
        data.first = reminders.value("title").toString();
        data.second = reminders.value("date").toDateTime();
        ReminderData.append(data);
    }

    reminders.endArray();

    bool dataChanged = false;
    for (int i = 0; i < ReminderData.count(); i++) {
        QPair<QString, QDateTime> data = ReminderData.at(i);
        if (data.second.msecsTo(QDateTime::currentDateTime()) > 0) {
            QVariantMap hints;
            hints.insert("category", "reminder.activate");
            hints.insert("sound-file", "qrc:/sounds/notifications/reminder.wav");
            notificationInterface->call("Notify", "theShell", (uint) 0, "theshell", "Reminder", data.first, QStringList(), hints, 30000);
            ReminderData.removeAt(i);
            i--;
            dataChanged = true;
        }
    }


    if (dataChanged) {
        reminders.beginWriteArray("reminders");
        int i = 0;
        for (QPair<QString, QDateTime> data : ReminderData) {
            reminders.setArrayIndex(i);
            reminders.setValue("title", data.first);
            reminders.setValue("date", data.second);
            i++;
        }
        reminders.endArray();
        reminders.endGroup();

        model->updateData();
    }
}

void RemindersPage::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
