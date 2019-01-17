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

#include "bthandsfree.h"

BTHandsfree::BTHandsfree(QWidget *parent) : QWidget(parent)
{
    layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setMargin(0);
    this->setLayout(layout);

    QFrame* lineSeperator = new QFrame;
    lineSeperator->setFixedWidth(2);
    lineSeperator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    lineSeperator->setFrameShape(QFrame::VLine);
    layout->addWidget(lineSeperator);

    infoLabel = new QLabel;
    layout->addWidget(infoLabel);

    hangupButton = new QPushButton;
    hangupButton->setFlat(true);
    hangupButton->setIcon(QIcon::fromTheme("call-stop"));
    hangupButton->setVisible(false);
    connect(hangupButton, &QPushButton::clicked, [=]() {
        hangUpButtonInterface->call("HangUp");
    });
    layout->addWidget(hangupButton);

    this->setVisible(false);

    QTimer* detector = new QTimer;
    detector->setInterval(1000);
    connect(detector, SIGNAL(timeout()), this, SLOT(detectDevices()));
    detector->start();
}

void BTHandsfree::detectDevices() {
    if (QDBusConnection::sessionBus().interface()->registeredServiceNames().value().contains("org.thesuite.tsbt")) {
        QStringList labelContent;
        QString labelOverride;

        QEventLoop loop;
        QDBusMessage getDevices = QDBusMessage::createMethodCall("org.thesuite.tsbt", "/org/thesuite/tsbt", "org.thesuite.tsbt", "handsfreeDevices");
        QDBusPendingReply<QStringList> reply = QDBusConnection::sessionBus().asyncCall(getDevices);
        while (!reply.isFinished()) {
            QApplication::processEvents();
        }

        if (reply.isValid()) {
            for (QString device : knownDevices) {
                if (!reply.value().contains(device)) {
                    interfaces.at(knownDevices.indexOf(device))->deleteLater();
                    interfaces.removeAt(knownDevices.indexOf(device));
                    knownDevices.removeAll(device);
                }
            }

            for (QString device : reply.value()) {
                if (!knownDevices.contains(device)) {
                    knownDevices.append(device);

                    QDBusInterface* interface = new QDBusInterface("org.thesuite.tsbt", "/org/thesuite/tsbt/handsfree/" + device, "org.thesuite.tsbt.Handsfree");
                    interfaces.append(interface);
                }
            }

            for (QDBusInterface* interface : interfaces) {
                QString callState = interface->property("CallState").toString();
                if (callState == "InCall") {
                    labelOverride = interface->property("DeviceName").toString();
                    labelOverride.append(" (" + tr("In call") + ")");
                    hangUpButtonInterface = interface;
                } else if (callState == "Dialling") {
                    labelOverride = interface->property("DeviceName").toString();
                    labelOverride.append(" (" + tr("Dialling...") + ")");
                    hangUpButtonInterface = interface;
                } else {
                    QString description;
                    description.append(interface->property("OperatorName").toString());
                    description.append(" (");
                    description.append(interface->property("DeviceName").toString());
                    description.append(")");
                    labelContent.append(description);
                }
            }

            if (knownDevices.count() == 0) {
                this->setVisible(false);
            } else {
                if (labelOverride == "") {
                    this->infoLabel->setText(labelContent.join(" · "));
                    hangupButton->setVisible(false);
                } else {
                    this->infoLabel->setText(labelOverride);
                    hangupButton->setVisible(true);
                }
                this->setVisible(true);
            }
        }
    } else {
        this->setVisible(false);
    }
}

QList<QString> BTHandsfree::getDevices() {
    QStringList retval;
    for (QDBusInterface* device : interfaces) {
        retval.append(device->property("DeviceName").toString());
    }
    return retval;
}

void BTHandsfree::placeCall(int deviceIndex, QString number) {
    interfaces.at(deviceIndex)->call(QDBus::NoBlock, "PlaceCall", number);
}
