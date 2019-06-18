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
#include "displaypane.h"
#include "ui_displaypane.h"

#include "redshiftengine.h"

#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QScroller>
#include <locationdaemon.h>

struct DisplayPanePrivate {
    QSettings settings;
    RedshiftEngine* redshift;

    uint redshiftSwitch;
};

DisplayPane::DisplayPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayPane)
{
    ui->setupUi(this);
    d = new DisplayPanePrivate();
    d->redshift = new RedshiftEngine();
    ui->usesLocation->setPixmap(QIcon::fromTheme("gps").pixmap(16 * theLibsGlobal::getDPIScaling(), 16 * theLibsGlobal::getDPIScaling()));

    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-display");

    int dpi = d->settings.value("screen/dpi", 96).toInt();
    switch (dpi) {
        case 96:
            ui->dpi100->setChecked(true);
            break;
        case 144:
            ui->dpi150->setChecked(true);
            break;
        case 192:
            ui->dpi200->setChecked(true);
            break;
        case 288:
            ui->dpi300->setChecked(true);
            break;
    }

    QString redshiftStart = d->settings.value("display/redshiftStart", "").toString();
    if (redshiftStart == "") {
        redshiftStart = ui->startRedshift->time().toString();
        d->settings.setValue("display/redshiftStart", redshiftStart);
    }
    ui->startRedshift->setTime(QTime::fromString(redshiftStart));

    QString redshiftEnd = d->settings.value("display/redshiftEnd", "").toString();
    if (redshiftEnd == "") {
        redshiftEnd = ui->endRedshift->time().toString();
        d->settings.setValue("display/redshiftEnd", redshiftEnd);
    }
    ui->endRedshift->setTime(QTime::fromString(redshiftEnd));

    ui->redshiftIntensity->setValue(d->settings.value("display/redshiftIntensity", 5000).toInt());
    ui->redshiftPause->setChecked(!d->settings.value("display/redshiftPaused", true).toBool());
    ui->sunlightRedshift->setChecked(d->settings.value("display/redshiftSunlightCycle", false).toBool());

    QLabel* snack = new QLabel();
    snack->setPixmap(QIcon::fromTheme("redshift-on").pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));
    QTimer::singleShot(0, [=] {
        //Register the Redshift switch
        sendMessage("register-switch", {tr("Redshift"), d->redshift->isEnabled(), "redshift"});

        //Register the snack
        sendMessage("register-snack", {QVariant::fromValue(snack)});
    });

    connect(d->redshift, &RedshiftEngine::redshiftEnabledChanged, [=](bool enabled) {
        sendMessage("set-switch", {d->redshiftSwitch, enabled});
        snack->setVisible(enabled);
    });

    QScroller::grabGesture(ui->scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
}

DisplayPane::~DisplayPane()
{
    d->redshift->deleteLater();
    delete d;
    delete ui;
}

QWidget* DisplayPane::mainWidget() {
    return this;
}

QString DisplayPane::name() {
    return tr("Display");
}

StatusCenterPaneObject::StatusPaneTypes DisplayPane::type() {
    return Setting;
}

int DisplayPane::position() {
    return 0;
}

void DisplayPane::message(QString name, QVariantList args) {
    if (name == "switch-registered") {
        if (args.at(1) == "redshift") {
            d->redshiftSwitch = args.at(0).toUInt();
        }
    } else if (name == "switch-toggled") {
        if (args.at(0) == d->redshiftSwitch) {
            d->redshift->overrideRedshift(args.at(1).toBool());
        }
    }
}

void DisplayPane::on_dpi100_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("screen/dpi", 96);
    }
}

void DisplayPane::on_dpi150_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("screen/dpi", 144);
    }
}

void DisplayPane::on_dpi200_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("screen/dpi", 192);
    }
}

void DisplayPane::on_dpi300_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("screen/dpi", 288);
    }
}

void DisplayPane::on_startRedshift_timeChanged(const QTime &time)
{
    d->settings.setValue("display/redshiftStart", time.toString());
    d->redshift->processTimer();
}

void DisplayPane::on_endRedshift_timeChanged(const QTime &time)
{
    d->settings.setValue("display/redshiftEnd", time.toString());
    d->redshift->processTimer();
}

void DisplayPane::on_redshiftIntensity_sliderMoved(int position)
{
    d->redshift->preview(position);
}

void DisplayPane::on_redshiftIntensity_sliderReleased()
{
    d->redshift->endPreview();
}

void DisplayPane::on_redshiftIntensity_valueChanged(int value)
{
    d->settings.setValue("display/redshiftIntensity", value);
}

void DisplayPane::on_redshiftPause_toggled(bool checked)
{
    d->redshift->processTimer();
    d->settings.setValue("display/redshiftPaused", !checked);
}

void DisplayPane::on_sunlightRedshift_toggled(bool checked)
{
    d->settings.setValue("display/redshiftSunlightCycle", checked);
    updateRedshiftTime();
}

void DisplayPane::updateRedshiftTime() {
    if (!d->settings.value("display/redshiftSunlightCycle", false).toBool()) {
        //Don't grab location if user doesn't want
        ui->startRedshift->setEnabled(true);
        ui->endRedshift->setEnabled(true);
        return;
    }

    ui->startRedshift->setEnabled(false);
    ui->endRedshift->setEnabled(false);

    LocationDaemon::singleShot()->then([=](Geolocation location) {
        updateRedshiftTime(location.latitude, location.longitude);
    });
}

void DisplayPane::updateRedshiftTime(double latitude, double longitude) {
    if (!d->settings.value("display/redshiftSunlightCycle", false).toBool()) {
        //Don't grab location if user doesn't want
        ui->startRedshift->setEnabled(true);
        ui->endRedshift->setEnabled(true);
        return;
    }

    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest sunriseApi(QUrl(QString("https://api.sunrise-sunset.org/json?lat=%1&lng=%2&formatted=0").arg(latitude).arg(longitude)));
    sunriseApi.setHeader(QNetworkRequest::UserAgentHeader, QString("theShell/%1").arg("9.0"));

    QNetworkReply* reply = manager->get(sunriseApi);
    connect(reply, &QNetworkReply::finished, [=] {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject root = doc.object();
        if (root.value("status").toString() != "OK") {
            qDebug() << root.value("status").toString();
            return;
        }

        //The time returned should be midway into the transition period, add/remove 30 minutes to compensate
        QJsonObject results = root.value("results").toObject();
        QDateTime sunrise = QDateTime::fromString(results.value("sunrise").toString(), Qt::ISODate).toLocalTime().addSecs(-1800);
        QDateTime sunset = QDateTime::fromString(results.value("sunset").toString(), Qt::ISODate).toLocalTime().addSecs(1800);

        ui->startRedshift->setDateTime(sunset);
        ui->endRedshift->setDateTime(sunrise);

        reply->deleteLater();
        manager->deleteLater();
    });
}

void DisplayPane::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}

void DisplayPane::on_displayPositioning_showDisplayPanel()
{
    sendMessage("show", QVariantList());
}
