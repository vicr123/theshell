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

#include "overview.h"
#include "ui_overview.h"

#include <QEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollArea>
#include <qmath.h>
#include <QTimer>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QVariantAnimation>
#include <QRandomGenerator>
#include <the-libs_global.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "Timers/timerpage.h"
#include "Stopwatch/stopwatchpage.h"
#include "Reminders/reminderspage.h"
#include "weatherengine.h"
#include <locationdaemon.h>

float getDPIScaling() {
    float currentDPI = QApplication::desktop()->logicalDpiX();
    return currentDPI / (float) 96;
}

Overview::Overview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Overview)
{
    ui->setupUi(this);

    this->weatherEngine = new WeatherEngine();

    this->informationalAttributes.lightColor = QColor(0, 150, 0);
    this->informationalAttributes.darkColor = QColor(0, 50, 0);

    ui->dstIcon->setPixmap(QIcon::fromTheme("chronometer").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
    ui->weatherIcon->setPixmap(QIcon::fromTheme("weather-clear").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
    ui->overviewLeftPane->installEventFilter(this);

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }

    QProcess* fullNameProc = new QProcess(this);
    fullNameProc->start("getent passwd " + name);
    fullNameProc->waitForFinished();
    QString parseName(fullNameProc->readAll());
    delete fullNameProc;

    QStringList nameParts = parseName.split(",").at(0).split(":");
    QString fullname;
    if (nameParts.count() > 4) {
        fullname = nameParts.at(4);
    }

    if (fullname == "") fullname = name;

    QTimer* timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [=] {
        ui->date->setText(QLocale().toString(QDateTime::currentDateTime(), "ddd dd MMM yyyy"));
        ui->time->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));

        QTime now = QTime::currentTime();
        if (now.hour() < 6) {
            ui->greetingLabel->setText(tr("Hi %1!").arg(fullname));
        } else if (now.hour() < 12) {
            ui->greetingLabel->setText(tr("Good morning, %1!").arg(fullname));
        } else if (now.hour() < 17) {
            ui->greetingLabel->setText(tr("Good afternoon, %1!").arg(fullname));
        } else {
            ui->greetingLabel->setText(tr("Good evening, %1!").arg(fullname));
        }
    });
    timer->start();

    animationTimer = new QTimer();
    animationTimer->setInterval(50);
    connect(animationTimer, SIGNAL(timeout()), ui->overviewLeftPane, SLOT(repaint()));

    randomObjectTimer = new QTimer();
    randomObjectTimer->setInterval(2000);
    connect(randomObjectTimer, &QTimer::timeout, [=] {
        if (QRandomGenerator::global()->bounded(20) == 3) {
            //Create a new plane
            Aircraft* a = new Aircraft();
            a->velocity = QRandomGenerator::global()->bounded(1, 3);
            a->location.setY(QRandomGenerator::global()->bounded(50));
            if (QRandomGenerator::global()->bounded(2)) {
                a->location.setX(ui->overviewLeftPane->width());
                a->velocity *= -1;
            } else {
                a->location.setX(0);
            }
            objects.append(a);
        }
    });

    connect(theLibsGlobal::instance(), &theLibsGlobal::powerStretchChanged, [=](bool isOn) {
        if (showing) {
            if (isOn) {
                animationTimer->stop();
                randomObjectTimer->stop();
            } else {
                animationTimer->start();
                randomObjectTimer->start();
            }
        }
    });

    TimerPage* timerPage = new TimerPage;
    connect(timerPage, &TimerPage::attenuate, [=] {
        sendMessage("attenuate", QVariantList() << true);
    });
    connect(timerPage, &TimerPage::deattenuate, [=] {
        sendMessage("attenuate", QVariantList() << false);
    });

    ui->rightStack->addWidget(timerPage);
    ui->rightStack->addWidget(new StopwatchPage);
    ui->rightStack->addWidget(new RemindersPage);

    updateDSTNotification();
    updateWeather();
}

Overview::~Overview()
{
    weatherEngine->deleteLater();
    delete ui;
}

QWidget* Overview::mainWidget() {
    return this;
}

QString Overview::name() {
    return tr("Overview");
}

StatusCenterPaneObject::StatusPaneTypes Overview::type() {
    return Informational;
}

int Overview::position() {
    return -1000;
}

void Overview::message(QString name, QVariantList args) {
    if (name == "show") {
        if (!theLibsGlobal::instance()->powerStretchEnabled()) {
            animationTimer->start();
            randomObjectTimer->start();
        }
        updateWeather();
    } else if (name == "hide") {
        animationTimer->stop();
        randomObjectTimer->stop();
    }
}

bool Overview::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->overviewLeftPane) {
        if (event->type() == QEvent::Paint) {
            QPaintEvent* e = (QPaintEvent*) event;
            QPainter p(ui->overviewLeftPane);
            p.setRenderHint(QPainter::Antialiasing);

            QColor newTextColor;

            QColor top, bottom;
            QTime now = QTime::currentTime();

            if (currentCondition.isValid && currentCondition.properties.value(WeatherCondition::isCloudy, false).toBool()) {
                top = QColor(150, 150, 150);
                bottom = QColor(100, 100, 100);
                newTextColor = QColor(Qt::black);
            } else if (now.hour() < 4 || now.hour() > 20) { //Assume night
                top = QColor(0, 36, 85);
                bottom = QColor(0, 17, 40);
                newTextColor = QColor(Qt::white);
            } else if (now.hour() > 8 && now.hour() < 16) { //Assume day
                top = QColor(126, 195, 255);
                bottom = QColor(64, 149, 185);
                newTextColor = QColor(Qt::black);
            } else { //Calculate interpolation
                int interpolation;
                //From 4-8 interpolate sunrise
                if (now.hour() > 4 && now.hour() < 8) {
                    interpolation = now.msecsSinceStartOfDay() - 14400000; //4 hours in milliseconds
                } else {
                    interpolation = 14400000 - (now.msecsSinceStartOfDay() - 57600000); //16 hours in milliseconds
                }

                QVariantAnimation a;
                a.setDuration(14400000);
                a.setCurrentTime(interpolation);
                a.setStartValue(QColor(0, 36, 85));
                a.setKeyValueAt(0.5, QColor(255, 140, 0));
                a.setEndValue(QColor(126, 195, 255));
                top = a.currentValue().value<QColor>();
                a.setStartValue(QColor(0, 17, 40));
                a.setKeyValueAt(0.5, QColor(167, 70, 25));
                a.setEndValue(QColor(64, 149, 185));
                bottom = a.currentValue().value<QColor>();

                bool dark = ((top.red() + top.green() + top.blue()) / 3) < 127;
                if (dark) {
                    newTextColor = QColor(Qt::white);
                } else {
                    newTextColor = QColor(Qt::black);
                }
            }

            QLinearGradient mainBackground;
            mainBackground.setStart(0, 0);
            mainBackground.setFinalStop(0, 500);
            mainBackground.setColorAt(0, top);
            mainBackground.setColorAt(1, bottom);

            p.setBrush(mainBackground);
            p.setPen(Qt::transparent);
            p.drawRect(0, 0, ui->overviewLeftPane->width(), ui->overviewLeftPane->height());

            //Draw background objects if neccessary
            if (currentCondition.isValid) {
                if (currentCondition.properties.value(WeatherCondition::isRainy, false).toBool()) drawRaindrops(&p);
                if (currentCondition.properties.value(WeatherCondition::WindBeaufort, 0).toInt() >= 4) drawWind(&p);
            }
            drawObjects(&p);

            //Draw celestial object if neccessary
            int daySegmentPassed = -1;
            bool isMoon = true;

            if (now.hour() < 5 || (now.hour() == 5 && now.minute() <= 30)) { //Draw moon
                daySegmentPassed = now.msecsSinceStartOfDay() + 19800000; //5.5 hours in milliseconds
            } else if (now.hour() > 18 || (now.hour() == 18 && now.minute() >= 30)) { //Draw moon
                daySegmentPassed = now.msecsSinceStartOfDay() - 66600000; //18.5 hours in milliseconds
            } else if ((now.hour() > 6 && now.hour() < 17) || (now.hour() == 6 && now.minute() >= 30) || (now.hour() == 17 && now.minute() <= 30)) { //Draw sun
                daySegmentPassed = now.msecsSinceStartOfDay() - 23400000; //6.5 hours in milliseconds
                isMoon = false;
            }

            if (isMoon) {
                p.setBrush(QColor(127, 127, 127));
            } else {
                //p.setBrush(QColor(255, 140, 0));
                p.setBrush(QColor(255, 224, 130));
            }
            if (daySegmentPassed >= 0) {
                float percentageThroughArc = (float) daySegmentPassed / (float) 39600000; //11 hours in milliseconds

                float sinArg = percentageThroughArc * M_PI;
                float heightDisplacement = -sin(sinArg) + 1;

                int top = (heightDisplacement * 100) + 70;
                int left = (ui->overviewLeftPaneScrollArea->width() + 100) * percentageThroughArc - 50;

                QPoint center(left, top);
                if (isMoon) {
                    p.drawEllipse(center, 30, 30);
                } else {
                    p.drawEllipse(center, 50, 50);
                }
            }


            QPalette pal = ui->overviewLeftPane->palette();
            pal.setColor(QPalette::WindowText, newTextColor);
            ui->overviewLeftPane->setPalette(pal);
        }
    }
    return false;
}

void Overview::launchDateTimeService() {
    QDBusMessage getMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListActivatableNames");
    QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(getMessage);
    if (!reply.value().contains("org.freedesktop.timedate1")) {
        return;
    }

    /*QDBusMessage launchMessage = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "StartServiceByName");
    QVariantList args;
    args.append("org.freedesktop.timedate1");
    args.append((uint) 0);
    launchMessage.setArguments(args);

    QDBusConnection::systemBus().call(launchMessage);*/

    QDBusConnection::systemBus().interface()->startService("org.freedesktop.timedate1");
}

void Overview::updateDSTNotification() {
    launchDateTimeService();

    QDBusInterface dateTimeInterface("org.freedesktop.timedate1", "/org/freedesktop/timedate1", "org.freedesktop.timedate1", QDBusConnection::systemBus());
    QString currentTimezone = dateTimeInterface.property("Timezone").toString();

    QString timezoneInfoPath = "/usr/share/zoneinfo/" + currentTimezone;
    QProcess* timezoneProcess = new QProcess();
    connect(timezoneProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus) {
        timezoneProcess->deleteLater();

        struct Changeover {
            QDateTime changeoverDate;
            bool isDST;
            int gmtOffset;
        };

        QList<Changeover> changeovers;

        while (!timezoneProcess->atEnd()) {
            QStringList parts = QString(timezoneProcess->readLine()).split(" ", QString::SkipEmptyParts);
            if (parts.length() == 16) {
                QStringList dateText;
                dateText.append(parts.at(4));
                //dateText.append(parts.at(2));

                //I am so sorry :(
                if (parts.at(2) == "Jan") {
                    dateText.append("01");
                } else if (parts.at(2) == "Feb") {
                    dateText.append("02");
                } else if (parts.at(2) == "Mar") {
                    dateText.append("03");
                } else if (parts.at(2) == "Apr") {
                    dateText.append("04");
                } else if (parts.at(2) == "May") {
                    dateText.append("05");
                } else if (parts.at(2) == "Jun") {
                    dateText.append("06");
                } else if (parts.at(2) == "Jul") {
                    dateText.append("07");
                } else if (parts.at(2) == "Aug") {
                    dateText.append("08");
                } else if (parts.at(2) == "Sep") {
                    dateText.append("09");
                } else if (parts.at(2) == "Oct") {
                    dateText.append("10");
                } else if (parts.at(2) == "Nov") {
                    dateText.append("11");
                } else if (parts.at(2) == "Dec") {
                    dateText.append("12");
                }

                dateText.append(parts.at(5));
                dateText.append(parts.at(3));

                Changeover c;
                QString dateConnectedText = dateText.join(" ");
                c.changeoverDate = QDateTime::fromString(dateConnectedText, "hh:mm:ss MM yyyy d");
                c.changeoverDate.setTimeSpec(Qt::UTC);
                c.isDST = parts.at(14).endsWith("1");
                c.gmtOffset = parts.at(15).mid(7).toInt();
                changeovers.append(c);
            }
        }

        bool showDaylightSavingsPanel = false;
        Changeover changeover;
        QDateTime current = QDateTime::currentDateTimeUtc();
        QDateTime currentLocal = QDateTime::currentDateTime();

        for (int i = 0; i < changeovers.count(); i++) {
            Changeover c = changeovers.at(i);

            int days = current.daysTo(c.changeoverDate);
            if (days > 0 && days < 14) {
                if ((currentLocal.isDaylightTime() && !c.isDST) || (!currentLocal.isDaylightTime() && c.isDST)) {
                    showDaylightSavingsPanel = true;
                    changeover = c;
                }
            }
        }

        if (showDaylightSavingsPanel) {
            ui->dstPanel->setVisible(true);
            ui->dstLabel->setText(tr("On %1, Daylight Savings Time will %2. The clock will automatically adjust %3 by %n hour(s).", nullptr, 1)
                                  .arg(QLocale().toString(changeover.changeoverDate.toLocalTime(), "ddd dd MMM yyyy"))
                                  //: This is used during Daylight Savings notifications and will appear as "On [date], Daylight Savings Time will (begin|end)".
                                  .arg(currentLocal.isDaylightTime() ? tr("end", "Context: \"Daylight Savings Time will end.\"") : tr("begin", "Context: \"Daylight Savings Time will begin.\""))
                                  //: This is used during Daylight Savings notifications and will appear as "The clock will automatically adjust (forwards|backwards) by [hours] hour(s).".
                                  .arg(currentLocal.isDaylightTime() ? tr("backwards", "Context: \"The clock will automatically adjust backwards\"") : tr("forwards", "Context: \"The clock will automatically adjust forwards\"")));
        } else {
            ui->dstPanel->setVisible(false);
        }
    });
    timezoneProcess->start("zdump -v " + timezoneInfoPath);
}

void Overview::drawRaindrops(QPainter* p) {
    p->save();
    while (raindrops.count() < 500) {
        Raindrop* r = new Raindrop();
        r->location.setY(0);
        r->location.setX(QRandomGenerator::global()->bounded(ui->overviewLeftPane->width() * 2));
        r->velocity = QRandomGenerator::global()->bounded(5, 20);
        /*r->horizontalDisplacement = 0;
        while (r->horizontalDisplacement == 0) {
            r->horizontalDisplacement = QRandomGenerator::global()->bounded(-5, 5);
        }*/
        r->horizontalDisplacement = -3;

        raindrops.append(r);
    }

    QList<Raindrop*> done;
    for (Raindrop* r : raindrops) {
        r->advance(ui->overviewLeftPane->height(), ui->overviewLeftPane->width());
        r->paint(p);
        if (r->done) done.append(r);
    }

    for (Raindrop* r : done) {
        raindrops.removeAll(r);
        delete r;
    }
    p->restore();
}

void Overview::drawWind(QPainter* p) {
    p->save();

    for (int i = 0; i < 3; i++) {
        if (QRandomGenerator::global()->bounded(4) == 2) {
            Wind* w = new Wind();
            w->location.setY(QRandomGenerator::global()->bounded(ui->overviewLeftPane->height()));
            w->location.setX(QRandomGenerator::global()->bounded(ui->overviewLeftPane->width() * 2) - ui->overviewLeftPane->width() / 2);
            w->scale = QRandomGenerator::global()->bounded((double) 3);
            w->timeScale = QRandomGenerator::global()->bounded(5) + 1;

            winds.append(w);
        }
    }

    QList<Wind*> done;
    for (Wind* w : winds) {
        w->advance(ui->overviewLeftPane->height(), ui->overviewLeftPane->width());
        w->paint(p);
        if (w->done) done.append(w);
    }

    for (Wind* w : done) {
        winds.removeAll(w);
        delete w;
    }
    p->restore();
}

void Overview::drawObjects(QPainter* p) {
    p->save();
    QList<BgObject*> done;
    for (BgObject* o : objects) {
        o->advance(ui->overviewLeftPane->height(), ui->overviewLeftPane->width());
        o->paint(p);
        if (o->done) done.append(o);
    }

    for (BgObject* r : done) {
        objects.removeAll(r);
        delete r;
    }
    p->restore();
}

BgObject::~BgObject() {

}

void Raindrop::advance(int maxHeight, int maxWidth) {
    horizontalDisplacement = velocity * tan(-0.26);
    this->location += QPoint(horizontalDisplacement, velocity);
    if (this->location.y() > maxHeight) done = true;
}

void Raindrop::paint(QPainter *p) {
    QPen pen;
    pen.setColor(QColor(0, 100, 255, 100));
    pen.setWidth(2);
    p->setPen(pen);
    p->drawLine(this->location, this->location - (QPoint(horizontalDisplacement, velocity) / 2));
}

void Aircraft::advance(int maxHeight, int maxWidth) {
    this->location.rx() += this->velocity;
    if (this->location.x() > maxWidth + 60 * getDPIScaling() || this->location.x() < -60 * getDPIScaling()) done = true;
}

void Aircraft::paint(QPainter *p) {
    QPen pen;
    pen.setColor(QColor(Qt::white));
    pen.setWidth(5);
    pen.setCapStyle(Qt::RoundCap);

    QLinearGradient g;
    g.setColorAt(0, QColor(255, 255, 255, 127));
    g.setColorAt(1, QColor(255, 255, 255, 0));

    p->setPen(pen);
    if (velocity < 0) {
        p->drawLine(this->location, this->location + QPoint(20, 0) * getDPIScaling());
        p->drawLine(this->location + QPoint(8, 0) * getDPIScaling(), this->location + QPoint(18, -4) * getDPIScaling());

        g.setStart(this->location + QPoint(25, 0) * getDPIScaling());
        g.setFinalStop(this->location + QPoint(60, 0) * getDPIScaling());
        pen.setBrush(g);
        p->setPen(pen);
        p->drawLine(this->location + QPoint(25, 0) * getDPIScaling(), this->location + QPoint(60, 0) * getDPIScaling());
    } else {
        p->drawLine(this->location, this->location + QPoint(-20, 0) * getDPIScaling());
        p->drawLine(this->location + QPoint(-8, 0) * getDPIScaling(), this->location + QPoint(-18, -4) * getDPIScaling());

        g.setStart(this->location + QPoint(-25, 0) * getDPIScaling());
        g.setFinalStop(this->location + QPoint(-60, 0) * getDPIScaling());
        pen.setBrush(g);
        p->setPen(pen);
        p->drawLine(this->location + QPoint(-25, 0) * getDPIScaling(), this->location + QPoint(-60, 0) * getDPIScaling());
    }
}

void Overview::updateWeather() {
    //The Yahoo API has been deprecated.
    ui->weatherPanel->setVisible(false);
    ui->metAttribLabel->setVisible(false);

    if (settings.value("overview/enableWeather", false).toBool()) {

        auto errorFunction = [=](QString error) {
            ui->weatherPanel->setVisible(false);
            ui->metAttribLabel->setVisible(false);
            currentCondition = WeatherCondition();
        };
        LocationDaemon::singleShot()->then([=](Geolocation location) {
            if (location.resolved) {
                weatherEngine->setCoordinates(location.latitude, location.longitude, qRound(location.altitude));
                weatherEngine->getCurrentWeather()->then([=](WeatherCondition condition) {
                    weatherEngine->getTodayWeather()->then([=](WeatherCondition todayCondition) {
                        LocationDaemon::reverseGeocode(location.latitude, location.longitude)->then([=](GeoPlace place) {
                            QString unit;
                            double currentTemp = condition.properties.value(WeatherCondition::CurrentTemp).toDouble();
                            double highTemp = todayCondition.properties.value(WeatherCondition::HighTemp).toDouble();
                            double lowTemp = todayCondition.properties.value(WeatherCondition::LowTemp).toDouble();

                            if (settings.value("overview/weatherInCelsius", true).toBool()) {
                                unit = "°C";
                            } else {
                                unit = "°F";
                                currentTemp = currentTemp * 9 / 5 + 32;
                                highTemp = highTemp * 9 / 5 + 32;
                                lowTemp = lowTemp * 9 / 5 + 32;
                            }

                            ui->weatherInfo->setText(tr("In %1, it's %2, with %3. Expect a high temperature of %4 and a low temperature of %5.")
                                                     .arg(place.name + ", " + place.administrativeName)
                                                     .arg(QString("%1%2").arg(qRound(currentTemp)).arg(unit))
                                                     .arg(condition.properties.value(WeatherCondition::Condition).toString())
                                                     .arg(QString("%1%2").arg(qRound(highTemp)).arg(unit))
                                                     .arg(QString("%1%2").arg(qRound(lowTemp)).arg(unit)));
                            ui->weatherPanel->setVisible(true);
                            ui->metAttribLabel->setVisible(true);

                            currentCondition = condition;
                        })->error(errorFunction);
                    })->error(errorFunction);
                })->error(errorFunction);
            } else {
                errorFunction("");
            }
        });

    }
}

void Wind::advance(int maxHeight, int maxWidth) {
    progress += timeScale;
    if (progress >= 100) {
        done = true;
    }
}

void Wind::paint(QPainter *p) {
    p->setPen(QColor(255, 255, 255, 255 - ((float) qBound(0, progress, 100) / 100 * 255)));
    p->drawLine(location.x() + progress * scale, location.y(), location.x() + progress * scale + progress * scale, location.y());
}

void Overview::on_timersButton_toggled(bool checked)
{
    if (checked) {
        ui->rightStack->setCurrentIndex(0);
    }
}

void Overview::on_stopwatchButton_toggled(bool checked)
{
    if (checked) {
        ui->rightStack->setCurrentIndex(1);
    }
}

void Overview::on_remindersButton_toggled(bool checked)
{
    if (checked) {
        ui->rightStack->setCurrentIndex(2);
    }
}

void Overview::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        qDebug() << "Retranslating Overview";
        ui->retranslateUi(this);
    }
}
