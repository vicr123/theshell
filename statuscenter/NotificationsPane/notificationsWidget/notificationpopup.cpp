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

#include "notificationpopup.h"
#include "ui_notificationpopup.h"

#include "notificationobject.h"

NotificationPopup* NotificationPopup::currentlyShowingPopup = NULL;
QList<NotificationPopup*> NotificationPopup::pendingPopups = QList<NotificationPopup*>();

NotificationPopup::NotificationPopup(int id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotificationPopup)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_X11NetWmWindowTypeNotification, true);
    this->setAttribute(Qt::WA_AcceptTouchEvents, true);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    ui->buttonsWidget->setFixedHeight(0);
    ui->downArrow->setPixmap(QIcon::fromTheme("go-down").pixmap(16 * theLibsGlobal::getDPIScaling(), 16 * theLibsGlobal::getDPIScaling()));
    ui->ContentsWidget->setFixedHeight(ui->bodyLabel->fontMetrics().height() + ui->ContentsWidget->layout()->contentsMargins().top());
    this->id = id;

    this->layout()->removeWidget(ui->mainWidget);

    dismisser = new QTimer();
    dismisser->setInterval(500);
    connect(dismisser, &QTimer::timeout, dismisser, [=] {
        if (timeoutLeft != -2000) {
            timeoutLeft -= 500;
            if (timeoutLeft < 0) {
                this->close();
                //emit notificationClosed(NotificationObject::Expired);
            }
        }
    });

    coverWidget = new QWidget();
    coverWidget->setParent(this);
    coverWidget->setAutoFillBackground(true);
    coverWidget->setVisible(false);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    coverWidget->setLayout(layout);

    layout->addStretch();

    coverAppIcon = new QLabel;
    layout->addWidget(coverAppIcon);

    coverAppName = new QLabel;
    QFont f = coverAppName->font();
    f.setPointSize(15);
    coverAppName->setFont(f);
    layout->addWidget(coverAppName);

    layout->addStretch();
}

NotificationPopup::~NotificationPopup()
{
    dismisser->deleteLater();
    coverWidget->deleteLater();
    coverAppIcon->deleteLater();
    coverAppName->deleteLater();
    delete ui;
}

void NotificationPopup::show() {
    if (currentlyShowingPopup != nullptr) {
        currentlyShowingPopup->close();
        //emit currentlyShowingPopup->notificationClosed(NotificationObject::Undefined);
        pendingPopups.append(this);
    } else {
        currentlyShowingPopup = this;
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        this->move(screenGeometry.topLeft().x(), screenGeometry.top() - this->height());
        this->setFixedWidth(screenGeometry.width());

        textHeight = ui->bodyLabel->fontMetrics().boundingRect(QRect(0, 0, screenGeometry.width() - this->layout()->contentsMargins().left() - this->layout()->contentsMargins().right(), 10000), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, ui->bodyLabel->text()).height();

        bool showDownArrow = false;
        if (textHeight > ui->bodyLabel->fontMetrics().height()) {
            showDownArrow = true;
        }
        if (actions.count() > 0) {
            showDownArrow = true;
        }
        ui->downContainer->setVisible(showDownArrow);

        this->setFixedHeight(ui->mainWidget->sizeHint().height());
        QDialog::show();

        tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");
        anim->setStartValue(this->geometry());
        anim->setEndValue(QRect(this->x(), screenGeometry.y(), this->width(), this->height()));
        anim->setDuration(500);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        anim->start();

        if (settings.value("notifications/emphasiseApp", true).toBool()) {
            coverWidget->move(0, 0);
            coverWidget->resize(this->width(), this->height() - 1);
            coverWidget->clearMask();
            coverWidget->setVisible(true);
            QTimer::singleShot(1000, [=] {
                tVariantAnimation* anim = new tVariantAnimation();
                QPoint origin = ui->appIcon->geometry().center();

                int radius = qSqrt(qPow(this->width() - origin.x(), 2) + qPow(this->height() - origin.y(), 2));
                anim->setStartValue(radius);
                anim->setEndValue(1);
                anim->setDuration(250);
                anim->setEasingCurve(QEasingCurve::InCubic);
                connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                    QRegion r(QRect(origin.x() - value.toInt(), origin.y() - value.toInt(), value.toInt() * 2, value.toInt() * 2), QRegion::Ellipse);
                    coverWidget->setMask(r);
                    this->repaint();
                    ui->mainWidget->repaint();
                });
                connect(anim, &tVariantAnimation::finished, [=] {
                    coverWidget->setVisible(false);
                });
                anim->start();

                dismisser->start();
            });
        } else {
            dismisser->start();
        }

        mouseEvents = true;
    }
}

void NotificationPopup::close() {
    dismisser->stop();

    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(this->x(), screenGeometry.y() - this->height(), this->width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(anim, &tPropertyAnimation::finished, [=] {
        QDialog::close();
        currentlyShowingPopup = nullptr;

        if (pendingPopups.count() > 0) {
            pendingPopups.takeFirst()->show();
        }
    });
    anim->start();

    mouseEvents = false;
}

void NotificationPopup::enterEvent(QEvent *event) {
    Q_UNUSED(event)

    if (mouseEvents) {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(ui->buttonsWidget->height());
        anim->setEndValue(ui->buttonsWidget->sizeHint().height());
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->buttonsWidget->setFixedHeight(value.toInt());
            this->setFixedHeight(ui->mainWidget->sizeHint().height());
        });
        anim->start();

        tVariantAnimation* anim2 = new tVariantAnimation();
        anim2->setStartValue(ui->downContainer->height());
        anim2->setEndValue(0);
        anim2->setDuration(250);
        anim2->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim2, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim2, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->downContainer->setFixedHeight(value.toInt());
        });
        anim2->start();

        tVariantAnimation* anim3 = new tVariantAnimation();
        anim3->setStartValue(ui->ContentsWidget->height());
        anim3->setEndValue(textHeight);
        anim3->setDuration(250);
        anim3->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim3, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim3, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->ContentsWidget->setFixedHeight(value.toInt() + ui->ContentsWidget->layout()->contentsMargins().top());
        });
        anim3->start();

        stopDismisser();
    }
}

void NotificationPopup::leaveEvent(QEvent *event) {
    Q_UNUSED(event)

    if (mouseEvents) {
        if (!this->rect().contains(mapFromGlobal(QCursor::pos()))) {
            tVariantAnimation* anim = new tVariantAnimation();
            anim->setStartValue(ui->buttonsWidget->height());
            anim->setEndValue(0);
            anim->setDuration(250);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->buttonsWidget->setFixedHeight(value.toInt());
                this->setFixedHeight(ui->mainWidget->sizeHint().height());
            });
            anim->start();

            tVariantAnimation* anim2 = new tVariantAnimation();
            anim2->setStartValue(ui->downContainer->height());
            anim2->setEndValue(ui->downContainer->sizeHint().height());
            anim2->setDuration(250);
            anim2->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim2, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim2, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->downContainer->setFixedHeight(value.toInt());
            });
            anim2->start();

            tVariantAnimation* anim3 = new tVariantAnimation();
            anim3->setStartValue(ui->ContentsWidget->height());
            anim3->setEndValue(ui->bodyLabel->fontMetrics().height());
            anim3->setDuration(250);
            anim3->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim3, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim3, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->ContentsWidget->setFixedHeight(value.toInt() + ui->ContentsWidget->layout()->contentsMargins().top());
            });
            anim3->start();

            startDismisser();
        }
    }
}

void NotificationPopup::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);

    if (currentTouch == 0) { //Dismissing by swiping to the right
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(200, 0, 0));
        painter.drawRect(0, 0, this->width(), this->height() - 1);

        QFont font = this->font();
        font.setPointSize(15);
        painter.setFont(font);

        QFontMetrics metrics(font);

        QRect textRect;
        textRect.setTop(0);
        textRect.setHeight(this->height() - 1);
        textRect.setWidth(metrics.width(tr("Dismiss")));
        if (ui->mainWidget->geometry().left() > textRect.width() + 40 * theLibsGlobal::getDPIScaling()) {
            textRect.moveRight(ui->mainWidget->geometry().left() - 20 * theLibsGlobal::getDPIScaling());
        } else {
            textRect.moveLeft(20 * theLibsGlobal::getDPIScaling());
        }

        QRect iconRect;
        iconRect.setWidth(16 * theLibsGlobal::getDPIScaling());
        iconRect.setHeight(16 * theLibsGlobal::getDPIScaling());
        iconRect.moveRight(textRect.left() - 9 * theLibsGlobal::getDPIScaling());
        iconRect.moveTop(this->height() / 2 - iconRect.height() / 2);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, tr("Dismiss"));
        painter.drawPixmap(iconRect, QIcon::fromTheme("arrow-right").pixmap(16 * theLibsGlobal::getDPIScaling(), 16 * theLibsGlobal::getDPIScaling()));
    }

    if (urgency != 1) {
        painter.setPen(Qt::transparent);
        if (urgency == 0) {
            painter.setBrush(QColor(0, 100, 255));
        } else {
            painter.setBrush(QColor(200, 0, 0));
        }
        painter.drawRect(0, 0, this->width(), 3 * theLibsGlobal::getDPIScaling());
        //painter.drawRect(0, 0, 3 * theLibsGlobal::getDPIScaling(), this->height());
    }


}

void NotificationPopup::setApp(QString appName, QIcon appIcon) {
    ui->appnameLabel->setText(appName);
    ui->appIcon->setPixmap(appIcon.pixmap(16 * theLibsGlobal::getDPIScaling(), 16 * theLibsGlobal::getDPIScaling()));

    QPixmap pm = appIcon.pixmap(32 * theLibsGlobal::getDPIScaling(), 32 * theLibsGlobal::getDPIScaling());
    coverAppIcon->setPixmap(pm);
    coverAppName->setText(appName);

    qulonglong red = 0, green = 0, blue = 0;

    int totalPixels = 0;
    QImage im = pm.toImage();
    for (int i = 0; i < pm.width(); i++) {
        for (int j = 0; j < pm.height(); j++) {
            QColor c = im.pixelColor(i, j);
            if (c.alpha() != 0) {
                //i = pm.width();
                //j = pm.height();

                //c.setAlpha(255);
                //QPalette pal = coverWidget->palette();
                //pal.setColor(QPalette::Window, c.darker(150));
                //coverWidget->setPalette(pal);
                red += c.red();
                green += c.green();
                blue += c.blue();
                totalPixels++;
            }
        }
    }

    QColor c;
    QPalette pal = coverWidget->palette();
    int averageCol = (pal.color(QPalette::Window).red() + pal.color(QPalette::Window).green() + pal.color(QPalette::Window).blue()) / 3;

    if (totalPixels == 0) {
        if (averageCol < 127) {
            c = pal.color(QPalette::Window).darker(200);
        } else {
            c = pal.color(QPalette::Window).lighter(200);
        }
    } else {
        c = QColor(red / totalPixels, green / totalPixels, blue / totalPixels);

        if (averageCol < 127) {
            c = c.darker(200);
        } else {
            c = c.lighter(200);
        }
    }


    pal.setColor(QPalette::Window, c);
    coverWidget->setPalette(pal);
}

void NotificationPopup::setSummary(QString summary) {
    ui->summaryLabel->setText(ui->summaryLabel->fontMetrics().elidedText(summary, Qt::ElideRight, 300 * theLibsGlobal::getDPIScaling()));
}

void NotificationPopup::setBody(QString body) {
    ui->bodyLabel->setText(body);
}

void NotificationPopup::setHints(QVariantMap hints) {
    this->hints = hints;

    if (hints.contains("urgency")) {
        this->urgency = hints.value("urgency").toInt();
    } else {
        this->urgency = 1;
    }
}

void NotificationPopup::setActions(QStringList actions, bool actionNamesAreIcons) {
    QBoxLayout* layout = (QBoxLayout*) ui->actionsWidget->layout();

    QLayoutItem* item = layout->takeAt(0);
    while (item != nullptr) {
        item->widget()->deleteLater();
        delete item;

        item = layout->takeAt(0);
    }

    if (actions.count() % 2 == 0) {
        for (int i = 0; i < actions.length(); i += 2) {
            QString key = actions.at(i);
            QString value = actions.at(i + 1);

            QPushButton* button = new QPushButton();
            button->setText(value);

            if (actionNamesAreIcons) {
                button->setIcon(QIcon::fromTheme(value));
            }

            connect(button, &QPushButton::clicked, [=] {
                emit actionClicked(key);

                if (!this->hints.value("resident", false).toBool()) {
                    this->close();
                }
            });
            layout->addWidget(button);

            this->actions.insert(key, value);
        }
    }
}

void NotificationPopup::setTimeout(int timeout) {
    if (timeout == 0) {
        timeoutLeft = -2000;
    } else {
        timeoutLeft = timeout;
    }
}

void NotificationPopup::on_dismissButton_clicked()
{
    this->close();
    emit notificationClosed(NotificationObject::Dismissed);
}

void NotificationPopup::setBigIcon(QIcon bigIcon) {
    ui->bigIconLabel->setPixmap(bigIcon.pixmap(32 * theLibsGlobal::getDPIScaling(), 32 * theLibsGlobal::getDPIScaling()));
}

bool NotificationPopup::event(QEvent* event) {
    if (event->type() == QEvent::TouchBegin) {
        QTouchEvent* e = (QTouchEvent*) event;

        touchStart = e->touchPoints().first().pos().toPoint();
        event->accept();
        stopDismisser();
        return true;
    } else if (event->type() == QEvent::TouchUpdate) {
        QTouchEvent* e = (QTouchEvent*) event;
        if (currentTouch == -1) {
            QPoint adjusted = e->touchPoints().first().pos().toPoint() - touchStart;
            if (adjusted.manhattanLength() > 5) {
                //Determine what's going on
                int angle;
                if (adjusted.x() == 0) {
                    if (adjusted.y() > 0) {
                        angle = 270;
                    } else {
                        angle = 90;
                    }
                } else {
                    float gradient = (float) -adjusted.y() / (float) adjusted.x();
                    angle = qRadiansToDegrees(atan(gradient));

                    if (adjusted.x() < 0) {
                        angle = 180 + angle;
                    } else if (angle < 0) {
                        angle = 360 + angle;
                    }
                }
                if (angle < 0) qDebug() << "Negative angle!!!!!! :(";


                if (angle < 30 || angle > 330) { //Swiping to the right to dismiss
                    qDebug() << "Swiping to dismiss";
                    currentTouch = 0;
                    touchStart = e->touchPoints().first().pos().toPoint();
                } else if (angle > 60 && angle < 120) { //Swiping up to hide
                    qDebug() << "Swiping to hide";
                    currentTouch = 1;
                    touchStart = e->touchPoints().first().pos().toPoint();
                }
            }
        } else if (currentTouch == 0) { //Swipe to dismiss
            QPoint adjusted = e->touchPoints().first().pos().toPoint() - touchStart;

            int left = adjusted.x();
            if (left < 0) left = 0;
            ui->mainWidget->move(left, 0);
            this->update();
        }
    } else if (event->type() == QEvent::TouchEnd) {
        if (currentTouch == 0) { //Swipe to dismiss
            if (ui->mainWidget->geometry().left() > this->width() / 4) {
                tVariantAnimation* anim = new tVariantAnimation();
                anim->setStartValue(ui->mainWidget->geometry().left());
                anim->setEndValue(ui->mainWidget->geometry().left() + this->width());
                anim->setDuration(500);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                    ui->mainWidget->move(value.toInt(), 0);
                    this->update();
                });
                connect(anim, &tVariantAnimation::finished, [=] {
                    anim->deleteLater();
                    ui->dismissButton->click();
                });
                anim->start();
            } else {
                tVariantAnimation* anim = new tVariantAnimation();
                anim->setStartValue(ui->mainWidget->geometry().left());
                anim->setEndValue(0);
                anim->setDuration(500);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                    ui->mainWidget->move(value.toInt(), 0);
                    this->update();
                });
                connect(anim, &tVariantAnimation::finished, [=] {
                    currentTouch = -1;
                    anim->deleteLater();
                });
                anim->start();
            }
        }
        startDismisser();
    } else if (event->type() == QEvent::TouchCancel) {
        currentTouch = -1;
        startDismisser();
    }
    return QDialog::event(event);
}

void NotificationPopup::resizeEvent(QResizeEvent* event) {
    ui->mainWidget->setFixedSize(this->size().width(), this->size().height() - 1);
}

void NotificationPopup::startDismisser() {
    dismisserStopCount--;
    if (dismisserStopCount == 0) dismisser->start();
}

void NotificationPopup::stopDismisser() {
    dismisserStopCount++;
    dismisser->stop();
}

void NotificationPopup::on_timeoutButton_clicked()
{
    stopDismisser();
    this->close();
}
