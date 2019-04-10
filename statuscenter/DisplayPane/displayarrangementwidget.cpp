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

#include <QPainter>
#include <QX11Info>
#include <QMouseEvent>
#include <QIcon>
#include <tnotification.h>

#include "ui_displayarrangementwidget.h"
#include "displayarrangementwidget.h"
#include "displayconfigurationwidget.h"

#include <X11/extensions/Xrandr.h>

const float ScreenScalingFactor = 10 / theLibsGlobal::getDPIScaling();

struct DisplayArrangementWidgetPrivate {
    RROutput output;

    XRRScreenResources* screenResources = nullptr;
    XRROutputInfo* outputInfo = nullptr;

    QRectF requestedGeometry;
    bool moved;
    bool primary = false;

    QPoint clickLocation;
    QPoint origin;

    DisplayConfigurationWidget* configurator;
};

DisplayArrangementWidget::DisplayArrangementWidget(RROutput output, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayArrangementWidget)
{
    ui->setupUi(this);
    d = new DisplayArrangementWidgetPrivate;

    ui->defaultLabel->setPixmap(QIcon::fromTheme("default").pixmap(QSize(16, 16) * theLibsGlobal::getDPIScaling()));

    d->output = output;

    d->configurator = new DisplayConfigurationWidget();
    connect(d->configurator, &DisplayConfigurationWidget::resolutionChanged, this, [=](QSize resolution) {
        d->requestedGeometry.setSize(QSizeF(resolution) / ScreenScalingFactor);
        this->resize(d->requestedGeometry.size().toSize());
    });
    connect(d->configurator, &DisplayConfigurationWidget::poweredChanged, this, [=](bool powered) {
        QPalette pal = QApplication::palette("QWidget");
        if (!powered) { //Display is off
            pal.setColor(QPalette::WindowText, pal.color(QPalette::Disabled, QPalette::WindowText));
            if (d->primary) emit setOtherDefault();
        }
        this->setPalette(pal);
    });
    connect(d->configurator, &DisplayConfigurationWidget::setDefault, [=] {
        emit setDefault();
        setDefaultOutput(true);
    });

    updateOutput();
}

DisplayArrangementWidget::~DisplayArrangementWidget()
{
    XRRFreeScreenResources(d->screenResources);
    d->configurator->deleteLater();

    delete d;
    delete ui;
}

bool DisplayArrangementWidget::checkNewOutput() {
    bool changedConfiguration = false;
    //Check if this output has just been connected or detached
    XRRScreenResources* newResources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    XRROutputInfo* newOutputInfo = XRRGetOutputInfo(QX11Info::display(), newResources, d->output);

    if (d->outputInfo->connection != newOutputInfo->connection) {
        //Connection state has changed
        if (newOutputInfo->connection == RR_Connected) {
            //This screen has just been connected
            tNotification* n = new tNotification();
            n->setSummary(tr("Screen %1 connected").arg(newOutputInfo->name));
            n->setText(tr("To start using it, configure your screens."));
            n->insertAction("configure", tr("Configure Screens"));
            n->setTransient(true);
            connect(n, &tNotification::actionClicked, [=] {
                emit showDisplayPanel();
            });
            n->post(false);

        } else if (newOutputInfo->connection == RR_Disconnected) {
            //This screen has just been disconnected
            //Turn off this screen

            d->configurator->setPowered(false);
            changedConfiguration = true;
            //XRRSetCrtcConfig(QX11Info::display(), newResources, newOutputInfo->crtc, CurrentTime, 0, 0, None, RR_Rotate_0, nullptr, 0);
        }
    }

    XRRFreeOutputInfo(newOutputInfo);
    XRRFreeScreenResources(newResources);

    if (!changedConfiguration) {
        updateOutput();
    }
    return changedConfiguration;
}

void DisplayArrangementWidget::updateOutput() {
    if (d->outputInfo != nullptr) XRRFreeOutputInfo(d->outputInfo);
    if (d->screenResources != nullptr) XRRFreeScreenResources(d->screenResources);
    d->screenResources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    d->outputInfo = XRRGetOutputInfo(QX11Info::display(), d->screenResources, d->output);

    ui->screenName->setText(QString::fromLatin1(d->outputInfo->name));
    d->configurator->setDisplayName(QString::fromLatin1(d->outputInfo->name));

    QMap<RRMode, XRRModeInfo> modes;
    for (int i = 0; i < d->screenResources->nmode; i++) {
        modes.insert(d->screenResources->modes[i].id, d->screenResources->modes[i]);
    }

    if (XRRGetOutputPrimary(QX11Info::display(), QX11Info::appRootWindow()) == d->output) {
        setDefaultOutput(true);
    } else {
        setDefaultOutput(false);
    }

    QList<XRRModeInfo> availableModes;
    for (int i = 0; i < d->outputInfo->nmode; i++) {
        availableModes.append(modes.value(d->outputInfo->modes[i]));
    }
    d->configurator->setModes(availableModes);

    if (d->outputInfo->crtc == 0) {
        d->configurator->setPowered(false);
        d->requestedGeometry = QRectF(0, 0, modes.first().width / ScreenScalingFactor, modes.first().height / ScreenScalingFactor);
        d->configurator->setCurrentMode(modes.first());
    } else {
        d->configurator->setPowered(true);
        XRRCrtcInfo* currentCrtc = XRRGetCrtcInfo(QX11Info::display(), d->screenResources, d->outputInfo->crtc);
        d->requestedGeometry = QRectF(QPointF(currentCrtc->x, currentCrtc->y) / ScreenScalingFactor, QSizeF(currentCrtc->width, currentCrtc->height) / ScreenScalingFactor);
        d->configurator->setCurrentMode(modes.value(currentCrtc->mode));
        XRRFreeCrtcInfo(currentCrtc);
    }
}

QRect DisplayArrangementWidget::requestedGeometry() {
    return d->requestedGeometry.toRect();
}

void DisplayArrangementWidget::doPosition(QPoint origin) {
    d->origin = origin;
    this->setGeometry(QRect(d->requestedGeometry.topLeft().toPoint() + origin, d->requestedGeometry.size().toSize()));
    ui->geom->setText(QString("%1,%2").arg(d->requestedGeometry.left() * ScreenScalingFactor).arg(d->requestedGeometry.top() * ScreenScalingFactor));
    this->setVisible(true);

    QPalette pal = QApplication::palette("QWidget");
    if (d->outputInfo->crtc == 0) { //Display is off
        pal.setColor(QPalette::WindowText, pal.color(QPalette::Disabled, QPalette::WindowText));
    }
    this->setPalette(pal);

    if (d->outputInfo->connection != RR_Connected) this->setVisible(false);
}

void DisplayArrangementWidget::paintEvent(QPaintEvent *paintEvent) {
    QPainter painter(this);
    painter.setBrush(this->palette().color(QPalette::Window));
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);
}

void DisplayArrangementWidget::mousePressEvent(QMouseEvent *event) {
    d->clickLocation = event->pos();
    d->moved = false;
    d->configurator->hide();
    this->raise();
}

void DisplayArrangementWidget::mouseMoveEvent(QMouseEvent *event) {
    move(mapToParent(event->pos() - d->clickLocation));
    d->requestedGeometry.moveTopLeft(this->geometry().topLeft() - d->origin);
    ui->geom->setText(QString("%1,%2").arg(d->requestedGeometry.left() * ScreenScalingFactor).arg(d->requestedGeometry.top() * ScreenScalingFactor));
    d->moved = true;
}

void DisplayArrangementWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (!d->moved) {
        emit configureMe(d->configurator);
    }
}

void DisplayArrangementWidget::set() {
    if (d->primary && XRRGetOutputPrimary(QX11Info::display(), QX11Info::appRootWindow()) != d->output) {
        XRRSetOutputPrimary(QX11Info::display(), QX11Info::appRootWindow(), d->output);
    }

    if (d->outputInfo->crtc == 0) {
        if (d->configurator->powered()) {
            //Find a suitable CRTC for this output
            RRCrtc crtc = None;
            for (int i = 0; i < d->outputInfo->ncrtc; i++) {
                struct XRRCrtcInfoDeleter {
                    static inline void cleanup(XRRCrtcInfo* pointer) {XRRFreeCrtcInfo(pointer);}
                };

                QScopedPointer<XRRCrtcInfo, XRRCrtcInfoDeleter> crtcInfo(XRRGetCrtcInfo(QX11Info::display(), d->screenResources, d->outputInfo->crtcs[i]));
                if (crtcInfo->noutput > 0) {
                    //This CRTC is already being used, but let's check if we can clone the displays
                    if (crtcInfo->mode != d->configurator->mode()) continue;
                    if (crtcInfo->x != d->requestedGeometry.left() * ScreenScalingFactor) continue;
                    if (crtcInfo->y != d->requestedGeometry.top() * ScreenScalingFactor) continue;
                    if (crtcInfo->rotation != RR_Rotate_0) continue;

                    //We can use this CRTC
                    crtc = d->outputInfo->crtcs[i];
                    break;
                } else {
                    //This CRTC is unused, so we can use this CRTC
                    crtc = d->outputInfo->crtcs[i];
                    break;
                }
            }

            if (crtc != None) {
                //Configure the output on this CRTC
                XRRSetCrtcConfig(QX11Info::display(), d->screenResources, crtc, CurrentTime, d->requestedGeometry.left() * ScreenScalingFactor, d->requestedGeometry.top() * ScreenScalingFactor, d->configurator->mode(), RR_Rotate_0, &d->output, 1);
            }
        } else {
            //Do nothing; the screen isn't powered and doesn't need to be powered
            return;
        }
    } else {
        if (d->configurator->powered()) {
            //Adjust this CRTC
            XRRSetCrtcConfig(QX11Info::display(), d->screenResources, d->outputInfo->crtc, CurrentTime, d->requestedGeometry.left() * ScreenScalingFactor, d->requestedGeometry.top() * ScreenScalingFactor, d->configurator->mode(), RR_Rotate_0, &d->output, 1);
        } else {
            //Turn off this CRTC
            XRRSetCrtcConfig(QX11Info::display(), d->screenResources, d->outputInfo->crtc, CurrentTime, 0, 0, None, RR_Rotate_0, nullptr, 0);
        }
    }
}

void DisplayArrangementWidget::offset(QPoint distance) {
    //Offset the displays so the top left is always 0,0
    d->requestedGeometry.moveTopLeft(d->requestedGeometry.topLeft() + distance);
    d->origin -= distance;
    ui->geom->setText(QString("%1,%2").arg(d->requestedGeometry.left() * ScreenScalingFactor).arg(d->requestedGeometry.top() * ScreenScalingFactor));
}

bool DisplayArrangementWidget::powered() {
    return d->configurator->powered();
}

void DisplayArrangementWidget::setDefaultOutput(bool isDefault) {
    d->primary = isDefault;
    d->configurator->setIsDefault(isDefault);
    ui->defaultLabel->setVisible(isDefault);
}
