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
                    labelOverride.append(" (In call)");
                    hangUpButtonInterface = interface;
                } else if (callState == "Dialling") {
                    labelOverride = interface->property("DeviceName").toString();
                    labelOverride.append(" (Dialling...)");
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
