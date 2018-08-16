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

Overview::Overview(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Overview)
{
    ui->setupUi(this);

    this->informationalAttributes.lightColor = QColor(0, 150, 0);
    this->informationalAttributes.darkColor = QColor(0, 50, 0);

    ui->dstIcon->setPixmap(QIcon::fromTheme("chronometer").pixmap(16, 16));//(16 * getDPIScaling(), 16 * getDPIScaling()));
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
            ui->greetingLabel->setText(tr("Good morning %1!").arg(fullname));
        } else if (now.hour() < 17) {
            ui->greetingLabel->setText(tr("Good afternoon %1!").arg(fullname));
        } else {
            ui->greetingLabel->setText(tr("Good evening %1!").arg(fullname));
        }
    });
    timer->start();

    /*QTimer* t = new QTimer();
    t->setInterval(50);
    connect(t, SIGNAL(timeout()), ui->overviewLeftPane, SLOT(repaint()));
    t->start();*/

    updateDSTNotification();
}

Overview::~Overview()
{
    delete ui;
}

QWidget* Overview::mainWidget() {
    return this;
}

QString Overview::name() {
    return "O'erview";
}

StatusCenterPaneObject::StatusPaneTypes Overview::type() {
    return Informational;
}

int Overview::position() {
    return 0;
}

void Overview::message(QString name, QVariantList args) {

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

            bool cloudy = false;
            if (cloudy) {
                top = QColor(150, 150, 150);
                bottom = QColor(100, 100, 100);
                newTextColor = QColor(Qt::black);
            } else if (now.hour() <= 4 || now.hour() >= 20) { //Assume night
                top = QColor(0, 36, 85);
                bottom = QColor(0, 17, 40);
                newTextColor = QColor(Qt::white);
            } else if (now.hour() >= 8 && now.hour() <= 16) { //Assume day
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
            if (cloudy) {
                drawRaindrops(&p);
            }

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

    for (Raindrop* r : raindrops) {
        if (r->advance(ui->overviewLeftPane->height())) {
            r->paint(p);
        } else {
            raindrops.removeOne(r);
            QTimer::singleShot(0, [=] {
                delete r;
            });
        }
    }
}

bool Raindrop::advance(int maxHeight) {
    horizontalDisplacement = velocity * tan(-0.26);
    this->location += QPoint(horizontalDisplacement, velocity);
    if (this->location.y() > maxHeight) {
        return false;
    }
    return true;
}

void Raindrop::paint(QPainter *p) {
    QPen pen;
    pen.setColor(QColor(0, 100, 255, 100));
    pen.setWidth(3);
    p->setPen(pen);
    p->drawLine(this->location, this->location - (QPoint(horizontalDisplacement, velocity) / 2));
}
