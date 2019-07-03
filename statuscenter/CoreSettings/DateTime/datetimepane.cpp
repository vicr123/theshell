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
#include "datetimepane.h"
#include "ui_datetimepane.h"

#include "timezonesmodel.h"
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QScroller>

#include <PolkitQt1/Authority>

struct DateTimePanePrivate {
    QSettings settings;

    QDBusInterface* interface;
    TimezonesModel* timezoneModel;
};

DateTimePane::DateTimePane(QWidget *parent) :
    tStackedWidget(parent),
    ui(new Ui::DateTimePane)
{
    ui->setupUi(this);
    d = new DateTimePanePrivate();
    d->interface = new QDBusInterface("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());


    this->setCurrentAnimation(SlideHorizontal);

    settingAttributes.icon = QIcon::fromTheme("preferences-system-time");

    //Ensure timedated is running
    launchDateTimeService()->then([=] {
        ui->ntpEnabledSwitch->setChecked(d->interface->property("NTP").toBool());
    });
    if (d->settings.value("time/use24hour", true).toBool()) {
        ui->twentyFourHourTimeButton->setChecked(true);
    } else {
        ui->twelveHourTimeButton->setChecked(true);
    }

    ui->datePicker->setPickOptions(tDateTimePicker::PickDate);
    ui->timePicker->setPickOptions(tDateTimePicker::PickTime);

    d->timezoneModel = new TimezonesModel(this);
    ui->timezonesList->setModel(d->timezoneModel);
    ui->timezonesList->setItemDelegate(new TimezonesModelDelegate());

    QScroller::grabGesture(ui->timezonesList, QScroller::LeftMouseButtonGesture);
}

DateTimePane::~DateTimePane()
{
    delete ui;
    delete d;
}

QWidget* DateTimePane::mainWidget() {
    return this;
}

QString DateTimePane::name() {
    return tr("Time and Date");
}

StatusCenterPaneObject::StatusPaneTypes DateTimePane::type() {
    return Setting;
}

int DateTimePane::position() {
    return 0;
}

void DateTimePane::message(QString name, QVariantList args) {

}


tPromise<void>* DateTimePane::launchDateTimeService() {
    return new tPromise<void>([=](QString& error) {
        QDBusMessage getMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListActivatableNames");
        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(getMessage);
        if (!reply.value().contains("org.freedesktop.timedate1")) {
            error = "timedated not installed";
            return;
        }

        QDBusReply<void> startReply = QDBusConnection::systemBus().interface()->startService("org.freedesktop.timedate1");
        if (reply.error().type() != QDBusError::NoError) {
            error = "Couldn't start timedated";
            return;
        }
    });
}

void DateTimePane::on_twentyFourHourTimeButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("time/use24hour", true);
    }
}

void DateTimePane::on_twelveHourTimeButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("time/use24hour", false);
    }
}

void DateTimePane::on_changeTimeButton_clicked()
{
    checkPolkit("org.freedesktop.timedate1.set-time")->then([=] {
        ui->datePicker->setDateTime(QDateTime::currentDateTime());
        ui->timePicker->setDateTime(QDateTime::currentDateTime());
        sendMessage(QStringLiteral("hide-menu"), QVariantList());
        this->setCurrentIndex(1);
    });
}

void DateTimePane::on_backButton_clicked()
{
    sendMessage(QStringLiteral("show-menu"), QVariantList());
    this->setCurrentIndex(0);
}

void DateTimePane::on_backButton_2_clicked()
{
    sendMessage(QStringLiteral("show-menu"), QVariantList());
    this->setCurrentIndex(0);
}

void DateTimePane::on_changeTimezoneButton_clicked()
{
    checkPolkit("org.freedesktop.timedate1.set-timezone")->then([=] {
        ui->searchTimezones->setText("");
        ui->timezonesList->selectionModel()->setCurrentIndex(d->timezoneModel->timezone(QTimeZone::systemTimeZone()), QItemSelectionModel::SelectCurrent);
        sendMessage(QStringLiteral("hide-menu"), QVariantList());
        this->setCurrentIndex(2);
    });
}

tPromise<void>* DateTimePane::checkPolkit(QString action) {
    return new tPromise<void>([=](QString& error) {
        //Check Polkit authorization
        PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync(action, PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::None);

        if (r == PolkitQt1::Authority::No) {
            error = "Polkit declined";
            return;
        } else if (r == PolkitQt1::Authority::Challenge) {
            sendMessage("hide", QVariantList());
            PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync(action, PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::AllowUserInteraction);
            sendMessage("show", QVariantList());
            if (r != PolkitQt1::Authority::Yes) {
                error = "Polkit declined";
                return;
            }
        }
    });
}

void DateTimePane::on_timezonesList_activated(const QModelIndex &index)
{
    //Ensure timedated is running
    QString tzId = index.data(Qt::UserRole).toString();
    launchDateTimeService()->then([=] {
        QDBusPendingCallWatcher* w = new QDBusPendingCallWatcher(d->interface->asyncCall("SetTimezone", tzId, false));
        connect(w, &QDBusPendingCallWatcher::finished, w, &QDBusPendingCallWatcher::deleteLater);

        sendMessage(QStringLiteral("show-menu"), QVariantList());
        this->setCurrentIndex(0);
    });
}

void DateTimePane::on_setDateTimeButton_clicked()
{
    //Ensure timedated is running
    QDateTime newTime(ui->datePicker->currentDateTime().date(), ui->timePicker->currentDateTime().time());
    qlonglong time = newTime.toMSecsSinceEpoch() * 1000;
    launchDateTimeService()->then([=] {
        QDBusPendingCallWatcher* w = new QDBusPendingCallWatcher(d->interface->asyncCall("SetTime", time, false, false));
        connect(w, &QDBusPendingCallWatcher::finished, w, &QDBusPendingCallWatcher::deleteLater);

        sendMessage(QStringLiteral("show-menu"), QVariantList());
        this->setCurrentIndex(0);
    });
}

void DateTimePane::on_ntpEnabledSwitch_toggled(bool checked)
{
    //Ensure we can only change the time manually if NTP is off
    ui->changeTimeButton->setEnabled(!checked);

    //Ensure timedated is running
    launchDateTimeService()->then([=] {
        if (checked != d->interface->property("NTP").toBool()) {
            checkPolkit("org.freedesktop.timedate1.set-ntp")->then([=] {
                QDBusPendingCallWatcher* w = new QDBusPendingCallWatcher(d->interface->asyncCall("SetNTP", checked, true));
                connect(w, &QDBusPendingCallWatcher::finished, w, &QDBusPendingCallWatcher::deleteLater);
            })->error([=](QString error) {
                Q_UNUSED(error)
                ui->ntpEnabledSwitch->setChecked(d->interface->property("NTP").toBool());
            });
        }
    });
}

void DateTimePane::on_searchTimezones_textChanged(const QString &arg1)
{
    d->timezoneModel->search(arg1);
}

void DateTimePane::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

