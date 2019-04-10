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
#include "displaypositionwidget.h"
#include "ui_displaypositionwidget.h"

#include <math.h>
#include <QX11Info>
#include <QDebug>
#include <QMessageBox>
#include <the-libs_global.h>

#include "nativeeventfilter.h"
#include "displayarrangementwidget.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

struct DisplayPositionWidgetPrivate {
    QRect dawSize;
    QList<DisplayArrangementWidget*> daws;

    QSettings settings;
    bool currentlyUpdating = false;
};

DisplayPositionWidget::DisplayPositionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DisplayPositionWidget)
{
    ui->setupUi(this);

    d = new DisplayPositionWidgetPrivate;

    ui->screensArea->installEventFilter(this);
    ui->screensArea->setFixedHeight(300 * theLibsGlobal::getDPIScaling());

    //Register for XRandR events
    QApplication::instance()->installNativeEventFilter(new NativeEventFilter(this));
    XRRSelectInput(QX11Info::display(), QX11Info::appRootWindow(), RRScreenChangeNotifyMask | RRCrtcChangeNotifyMask | RROutputChangeNotifyMask | RROutputPropertyNotifyMask);
    loadDisplays();
}

DisplayPositionWidget::~DisplayPositionWidget()
{
    delete d;
    delete ui;
}

void DisplayPositionWidget::loadDisplays() {
    //Generate all DAWs and calculate encompassing rectangle
    XRRMonitorInfo* monitorInfo;
    int monitorCount;

    XRRScreenResources* resources = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    monitorInfo = XRRGetMonitors(QX11Info::display(), QX11Info::appRootWindow(), False, &monitorCount);

    for (int i = 0; i < resources->noutput; i++) {
        DisplayArrangementWidget* daw = new DisplayArrangementWidget(resources->outputs[i], ui->screensContent);
        connect(this, &DisplayPositionWidget::repositionDisplays, daw, &DisplayArrangementWidget::doPosition);
        connect(daw, &DisplayArrangementWidget::configureMe, [=](QWidget* configurator) {
            QPoint topLeftGeom = ui->screensContent->mapTo(this, daw->geometry().topRight()) + QPoint(10,0);
            configurator->setParent(this);
            configurator->move(topLeftGeom);
            if (configurator->geometry().right() > this->width()) {
                topLeftGeom = ui->screensContent->mapTo(this, daw->geometry().topLeft()) - QPoint(10 + configurator->width(), 0);
                configurator->move(topLeftGeom);
            }
            configurator->show();
            configurator->raise();
        });
        connect(this, &DisplayPositionWidget::setDefault, daw, [=] {
            daw->setDefaultOutput(false);
        });
        connect(daw, &DisplayArrangementWidget::setDefault, this, &DisplayPositionWidget::setDefault);
        connect(daw, &DisplayArrangementWidget::setOtherDefault, this, [=] {
            emit setDefault();
            for (DisplayArrangementWidget* daw : d->daws) {
                if (daw->powered()) {
                    daw->setDefaultOutput(true);
                    return;
                }
            }
        });
        connect(daw, &DisplayArrangementWidget::showDisplayPanel, this, &DisplayPositionWidget::showDisplayPanel);
        d->dawSize = d->dawSize.united(daw->requestedGeometry());
        d->daws.append(daw);
    }

    XRRFreeScreenResources(resources);
    XRRFreeMonitors(monitorInfo);

    //Move the encompassing rectangle to the middle
    d->dawSize.moveCenter(QPoint(ui->screensArea->width() / 2, ui->screensArea->height() / 2));

    //Render each DAW
    emit repositionDisplays(d->dawSize.topLeft());
}

void DisplayPositionWidget::reloadDisplays() {
    if (d->currentlyUpdating) return; //Don't do anything if we're currently updating our displays

    d->currentlyUpdating = true;
    qDebug() << "RandR changed!";
    bool configurationNeedsSet = false;
    for (DisplayArrangementWidget* daw : d->daws) {
        if (daw->checkNewOutput()) configurationNeedsSet = true;
    }

    if (configurationNeedsSet) {
        //Set the requested screen configuration
        d->currentlyUpdating = false;
        ui->setButton->click();
        d->currentlyUpdating = true;

        //Manually update each output
        for (DisplayArrangementWidget* daw : d->daws) {
            daw->updateOutput();
        }
    }

    emit repositionDisplays(d->dawSize.topLeft());
    d->currentlyUpdating = false;
}

bool DisplayPositionWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->screensArea) {
        if (event->type() == QEvent::Resize) {
            //Move the encompassing rectangle to the middle
            d->dawSize.moveCenter(QPoint(ui->screensArea->width() / 2, ui->screensArea->height() / 2));

            //Render each DAW
            emit repositionDisplays(d->dawSize.topLeft());
        }
    }
    return false;
}

void DisplayPositionWidget::on_setButton_clicked()
{
    //Perform some sanity checks
    bool havePoweredDisplay = false;
    for (DisplayArrangementWidget* daw : d->daws) {
        if (daw->powered()) havePoweredDisplay = true;
    }

    if (!havePoweredDisplay) {
        if (QMessageBox::warning(this, tr("No Powered Displays"), tr("All available displays in your configuration are off. Are you sure you want to set this screen configuration?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
            //Bail out
            return;
        }
    }

    //We're updating now
    d->currentlyUpdating = true;

    QRect rootWindowRect;
    for (DisplayArrangementWidget* daw : d->daws) {
        if (daw->powered()) {
            rootWindowRect = rootWindowRect.united(daw->requestedGeometry());
        }
    }

    //Reset all the screens so that top left is always 0,0
    QPoint distance = -rootWindowRect.topLeft();
    for (DisplayArrangementWidget* daw : d->daws) {
        daw->offset(distance);
        daw->set();
    }

    rootWindowRect.setSize(rootWindowRect.size() *= 10);

    int dpi = d->settings.value("screen/dpi", 96).toInt();
    //qDebug() << "XRRSetScreenSize" << rootWindowRect.width() << rootWindowRect.height() << (25.4 * rootWindowRect.width()) / dpi << (25.4 * rootWindowRect.height()) / dpi;
    XRRSetScreenSize(QX11Info::display(), QX11Info::appRootWindow(), rootWindowRect.width(), rootWindowRect.height(), round((25.4 * rootWindowRect.width()) / dpi), round((25.4 * rootWindowRect.height()) / dpi));

    d->currentlyUpdating = false;
}

void DisplayPositionWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
