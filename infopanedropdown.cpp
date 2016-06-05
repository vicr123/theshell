#include "infopanedropdown.h"
#include "ui_infopanedropdown.h"

extern void playSound(QUrl, bool = false);
extern QIcon getIconFromTheme(QString name, QColor textColor);
extern void EndSession(EndSessionWait::shutdownType type);

InfoPaneDropdown::InfoPaneDropdown(NotificationDBus* notificationEngine, UPowerDBus* powerEngine, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoPaneDropdown)
{
    ui->setupUi(this);

    this->notificationEngine = notificationEngine;
    this->powerEngine = powerEngine;
    notificationEngine->setDropdownPane(this);

    connect(notificationEngine, SIGNAL(newNotification(int,QString,QString,QIcon)), this, SLOT(newNotificationReceived(int,QString,QString,QIcon)));
    connect(notificationEngine, SIGNAL(removeNotification(int)), this, SLOT(removeNotification(int)));
    connect(this, SIGNAL(closeNotification(int)), notificationEngine, SLOT(CloseNotificationUserInitiated(int)));

    connect(powerEngine, SIGNAL(batteryChanged(int)), this, SLOT(batteryLevelChanged(int)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    timer->setInterval(1000);
    timer->start();

    ui->label_7->setVisible(false);
    ui->pushButton_3->setVisible(false);
    ui->label_10->setText("Coming soon. For now, use 'wifi-menu' in a terminal. Sorry for the inconvenience...");
    ui->pushButton_4->setEnabled(false);
    ui->lineEdit->setEnabled(false);
    ui->listWidget->setEnabled(false);
    //ui->label_22->setPixmap(getIconFromTheme("flight.svg", this->palette().color(QPalette::Window)).pixmap(16, 16));
    ui->FlightSwitch->setOnIcon(getIconFromTheme("flight.svg", this->palette().color(QPalette::Window)));


    if (!QFile("/usr/bin/systemsettings5").exists()) {
        ui->pushButton_8->setVisible(false);
    }

    QString redshiftStart = settings.value("display/redshiftStart", "").toString();
    if (redshiftStart == "") {
        redshiftStart = ui->startRedshift->time().toString();
        settings.setValue("display/redshiftStart", redshiftStart);
    }
    ui->startRedshift->setTime(QTime::fromString(redshiftStart));

    QString redshiftEnd = settings.value("display/redshiftEnd", "").toString();
    if (redshiftEnd == "") {
        redshiftEnd = ui->endRedshift->time().toString();
        settings.setValue("display/redshiftEnd", redshiftEnd);
    }
    ui->endRedshift->setTime(QTime::fromString(redshiftEnd));

    QString redshiftVal = settings.value("display/redshiftIntensity", "").toString();
    if (redshiftVal == "") {
        redshiftVal = ui->endRedshift->time().toString();
        settings.setValue("display/redshiftIntensity", redshiftVal);
    }
    ui->redshiftIntensity->setValue(redshiftVal.toInt());

    QString thewaveVoiceEngine = settings.value("thewave/ttsEngine", "festival").toString();
    if (thewaveVoiceEngine == "pico2wave") {
        ui->thewaveTTSpico2wave->setChecked(true);
    } else if (thewaveVoiceEngine == "espeak") {
        ui->thewaveTTSespeak->setChecked(true);
    } else if (thewaveVoiceEngine == "festival") {
        ui->thewaveTTSfestival->setChecked(true);
    }

    ui->lineEdit_2->setText(settings.value("startup/autostart", "").toString());
    ui->redshiftPause->setChecked(!settings.value("display/redshiftPaused", true).toBool());
    ui->TouchFeedbackSwitch->setChecked(settings.value("input/touchFeedbackSound", false).toBool());
    ui->thewaveWikipediaSwitch->setChecked(settings.value("thewave/wikipediaSearch", true).toBool());
    ui->thewaveOffensiveSwitch->setChecked(settings.value("thewave/blockOffensiveWords", true).toBool());
    ui->theWaveName->setText(settings.value("thewave/name", "").toString());

    eventTimer = new QTimer(this);
    eventTimer->setInterval(1000);
    connect(eventTimer, SIGNAL(timeout()), this, SLOT(processTimer()));
    eventTimer->start();

    UGlobalHotkeys* exitKey = new UGlobalHotkeys(this);
    exitKey->registerHotkey("Alt+F7");
    connect(exitKey, SIGNAL(activated(size_t)), this, SLOT(on_pushButton_clicked()));

    QObjectList allObjects;
    allObjects.append(this);

    /*do {
       for (QObject* object : allObjects) {
           if (object != NULL) {
               if (object->children().count() != 0) {
                   for (QObject* object2 : object->children()) {
                       allObjects.append(object2);
                   }
               }
               object->installEventFilter(this);
               allObjects.removeOne(object);
           }
       }
    } while (allObjects.count() != 0);*/


}

InfoPaneDropdown::~InfoPaneDropdown()
{
    delete ui;
}

void InfoPaneDropdown::processTimer() {
    QTime time = QTime::currentTime();
    if (ui->redshiftPause->isChecked() && time.secsTo(ui->startRedshift->time()) <= 0 && time.secsTo(ui->endRedshift->time()) >= 0) {
        if (!isRedshiftOn) {
            isRedshiftOn = true;
            QProcess::startDetached("redshift -O " + QString::number(ui->redshiftIntensity->value()));
        }
    } else {
        if (isRedshiftOn) {
            isRedshiftOn = false;
            QProcess::startDetached("redshift -O 6500");
        }
    }

    {
        cups_dest_t *destinations;
        int destinationCount = cupsGetDests(&destinations);

        for (int i = 0; i < destinationCount; i++) {
            cups_dest_t currentDestination = destinations[i];

            if (!printersFrames.keys().contains(currentDestination.name)) {
                QFrame* frame = new QFrame();
                QHBoxLayout* layout = new QHBoxLayout();
                layout->setMargin(0);
                frame->setLayout(layout);

                QFrame* statFrame = new QFrame();
                QHBoxLayout* statLayout = new QHBoxLayout();
                statLayout->setMargin(0);
                statFrame->setLayout(statLayout);
                layout->addWidget(statFrame);

                QLabel* iconLabel = new QLabel();
                QPixmap icon = QIcon::fromTheme("printer").pixmap(22, 22);
                if (currentDestination.is_default) {
                    QPainter *p = new QPainter();
                    p->begin(&icon);
                    p->drawPixmap(10, 10, 12, 12, QIcon::fromTheme("emblem-checked").pixmap(12, 12));
                    p->end();
                }
                iconLabel->setPixmap(icon);
                statLayout->addWidget(iconLabel);

                QLabel* nameLabel = new QLabel();
                nameLabel->setText(currentDestination.name);
                QFont font = nameLabel->font();
                font.setBold(true);
                nameLabel->setFont(font);
                statLayout->addWidget(nameLabel);

                QLabel* statLabel = new QLabel();
                statLabel->setText("Idle");
                statLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                statLayout->addWidget(statLabel);

                /*QPushButton* button = new QPushButton();
                button->setIcon(QIcon::fromTheme("window-close"));
                connect(button, &QPushButton::clicked, [=]() {
                    emit closeNotification(id);
                });
                layout->addWidget(button);*/

                ui->printersList->layout()->addWidget(frame);
                printersFrames.insert(currentDestination.name, frame);
                printersStatFrames.insert(currentDestination.name, frame);
                printersStats.insert(currentDestination.name, statLabel);
            }

            QString state = "";
            QString stateReasons = "";
            for (int i = 0; i < currentDestination.num_options; i++) {
                cups_option_t currentOption = currentDestination.options[i];

                if (strncmp(currentOption.name, "printer-state", strlen(currentOption.name)) == 0) {
                    if (strncmp(currentOption.value, "3", 1) == 0) {
                        state = "Idle";
                        printersStatFrames.value(currentDestination.name)->setEnabled(true);
                    } else if (strncmp(currentOption.value, "4", 1) == 0) {
                        state = "Printing";
                        printersStatFrames.value(currentDestination.name)->setEnabled(true);
                    } else if (strncmp(currentOption.value, "5", 1) == 0) {
                        state = "Stopped";
                        printersStatFrames.value(currentDestination.name)->setEnabled(false);
                    }
                } else if (strncmp(currentOption.name, "printer-state-reasons", strlen(currentOption.name)) == 0) {
                    stateReasons = QString::fromUtf8(currentOption.value, strlen(currentOption.value));
                }
            }
            printersStats.value(currentDestination.name)->setText(state + " / " + stateReasons);

        }

        cupsFreeDests(destinationCount, destinations);
    }
}

void InfoPaneDropdown::timerTick() {
    ui->date->setText(QDateTime::currentDateTime().toString("ddd dd MMM yyyy"));
    ui->time->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));

}

void InfoPaneDropdown::show(dropdownType showWith) {
    changeDropDown(showWith);
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    this->setGeometry(screenGeometry.x(), screenGeometry.y() - screenGeometry.height(), screenGeometry.width(), screenGeometry.height());

    QDialog::show();

    QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
    a->setStartValue(this->geometry());
    a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), this->height()));
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->setDuration(500);
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
    a->start();
    //QDialog::showFullScreen();
}

void InfoPaneDropdown::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
    a->setStartValue(this->geometry());
    a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - screenGeometry.height(), this->width(), this->height()));
    a->setEasingCurve(QEasingCurve::OutCubic);
    a->setDuration(500);
    connect(a, &QPropertyAnimation::finished, [=]() {
        QDialog::close();
    });
    connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
    a->start();
}

void InfoPaneDropdown::changeDropDown(dropdownType changeTo) {
    //QFrame *currentDropDownFrame;
    this->currentDropDown = changeTo;

    ui->networkFrame->setVisible(false);
    ui->batteryFrame->setVisible(false);
    ui->notificationsFrame->setVisible(false);
    ui->clockFrame->setVisible(false);
    ui->settingsFrame->setVisible(false);
    ui->printFrame->setVisible(false);

    ui->clockLabel->setShowDisabled(true);
    ui->batteryLabel->setShowDisabled(true);
    ui->notificationsLabel->setShowDisabled(true);
    ui->networkLabel->setShowDisabled(true);
    ui->printLabel->setShowDisabled(true);

    switch (changeTo) {
    case Settings:
        ui->settingsFrame->setVisible(true);

        break;
    case Clock:
        ui->clockFrame->setVisible(true);
        ui->clockLabel->setShowDisabled(false);
        break;
    case Battery:
        ui->batteryFrame->setVisible(true);
        ui->batteryLabel->setShowDisabled(false);
        break;
    case Notifications:
        ui->notificationsFrame->setVisible(true);
        ui->notificationsLabel->setShowDisabled(false);
        break;
    case Network:
        ui->networkFrame->setVisible(true);
        ui->networkLabel->setShowDisabled(false);

        getNetworks();
        break;
    case Print:
        ui->printFrame->setVisible(true);
        ui->printLabel->setShowDisabled(false);

    }

    if (changeTo == 0) {
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(true);
    } else if (changeTo == 4) {
        ui->pushButton_5->setEnabled(true);
        ui->pushButton_6->setEnabled(false);
    } else if (changeTo == -1) {
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(false);
    } else {
        ui->pushButton_5->setEnabled(true);
        ui->pushButton_6->setEnabled(true);
    }
}

void InfoPaneDropdown::resizeEvent(QResizeEvent *event) {
    event->accept();
    QSize size = event->size();

    //ui->clockFrame->resize(size);

}

void InfoPaneDropdown::on_pushButton_clicked()
{
    this->close();
}

void InfoPaneDropdown::getNetworks() {
    QDBusInterface *i = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus(), this);
    QDBusReply<QList<QDBusObjectPath>> reply = i->call("GetDevices");

    if (reply.isValid()) {
        for (QDBusObjectPath device : reply.value()) {
            QDBusInterface *i = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus(), this);
            switch (i->property("DeviceType").toInt()) {
            case 1: //Ethernet

                break;
            case 2: //WiFi
                if (i->property("State").toInt() == 100) { //Connected to a network
                    QDBusInterface *wifi = new QDBusInterface("org.freedesktop.NetworkManager", device.path(), "org.freedesktop.NetworkManager.Device.Wireless", QDBusConnection::systemBus(), this);
                    QDBusInterface *ap = new QDBusInterface("org.freedesktop.NetworkManager", wifi->property("ActiveAccessPoint").value<QDBusObjectPath>().path(), "org.freedesktop.NetworkManager.AccessPoint", QDBusConnection::systemBus(), this);
                    emit networkLabelChanged("Connected to " + ap->property("Ssid").toString());
                } else {
                    emit networkLabelChanged("Disconnected from the internet.");
                }
                break;
            }
        }
    } else {

    }


}

void InfoPaneDropdown::on_pushButton_5_clicked()
{
    changeDropDown(dropdownType(currentDropDown - 1));
}

void InfoPaneDropdown::on_pushButton_6_clicked()
{
    changeDropDown(dropdownType(currentDropDown + 1));
}

void InfoPaneDropdown::on_clockLabel_clicked()
{
    changeDropDown(Clock);
}

void InfoPaneDropdown::on_batteryLabel_clicked()
{
    changeDropDown(Battery);
}

void InfoPaneDropdown::on_networkLabel_clicked()
{
    changeDropDown(Network);
}

void InfoPaneDropdown::on_notificationsLabel_clicked()
{
    changeDropDown(Notifications);
}

void InfoPaneDropdown::startTimer(QTime time) {
    if (timer != NULL) { //Check for already running timer
        timer->stop();
        delete timer;
        timer = NULL;
        ui->timeEdit->setVisible(true);
        ui->label_7->setVisible(false);
        ui->label_7->setEnabled(true);
        ui->pushButton_2->setText("Start");
        ui->pushButton_3->setVisible(false);
        emit timerVisibleChanged(false);
        emit timerEnabledChanged(true);
    }
    ui->pushButton_2->setText("Pause");
    timeUntilTimeout = time;
    ui->label_7->setText(ui->timeEdit->text());
    ui->timeEdit->setVisible(false);
    ui->label_7->setVisible(true);
    timer = new QTimer();
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [=]() {
        timeUntilTimeout = timeUntilTimeout.addSecs(-1);
        if (timeUntilTimeout == QTime(0, 0, 0)) {
                   notificationEngine->Notify("theShell", 0, "", "Timer Elapsed",
                                              "Your timer has completed.",
                                              QStringList(), QVariantMap(), 0);
                   ui->timeEdit->setVisible(true);
                   ui->label_7->setVisible(false);
                   ui->pushButton_2->setText("Start");
                   playSound(QUrl::fromLocalFile("/usr/share/sounds/contemporary/alarm1.ogg"));
                   timer->stop();
                   delete timer;
                   timer = NULL;
                   emit timerVisibleChanged(false);
        } else {
            ui->label_7->setText(timeUntilTimeout.toString("HH:mm:ss"));
            emit timerChanged(timeUntilTimeout.toString("HH:mm:ss"));
        }
    });
    timer->start();
    emit timerChanged(timeUntilTimeout.toString("HH:mm:ss"));
}

void InfoPaneDropdown::on_pushButton_2_clicked()
{
    if (timer == NULL) {
        startTimer(ui->timeEdit->time());
    } else {
        if (timer->isActive()) {
            timer->stop();
            ui->pushButton_3->setVisible(true);
            ui->label_7->setEnabled(false);
            ui->pushButton_2->setText("Resume");
            emit timerEnabledChanged(false);
        } else {
            timer->start();
            ui->pushButton_3->setVisible(false);
            ui->label_7->setEnabled(true);
            ui->pushButton_2->setText("Pause");
            emit timerEnabledChanged(true);
        }
    }
}

void InfoPaneDropdown::on_pushButton_3_clicked()
{
    delete timer;
    timer = NULL;
    ui->timeEdit->setVisible(true);
    ui->label_7->setVisible(false);
    ui->label_7->setEnabled(true);
    ui->pushButton_2->setText("Start");
    ui->pushButton_3->setVisible(false);
    emit timerVisibleChanged(false);
    emit timerEnabledChanged(true);
}

void InfoPaneDropdown::on_pushButton_7_clicked()
{
    changeDropDown(Settings);
}

void InfoPaneDropdown::on_pushButton_8_clicked()
{
    QProcess::startDetached("systemsettings5");
    this->close();
}

void InfoPaneDropdown::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void InfoPaneDropdown::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void InfoPaneDropdown::on_lineEdit_2_editingFinished()
{
    settings.setValue("startup/autostart", ui->lineEdit_2->text());
}

void InfoPaneDropdown::on_resolutionButton_clicked()
{
    QProcess::startDetached("kcmshell5 kcm_kscreen");
    this->close();
}

void InfoPaneDropdown::on_startRedshift_timeChanged(const QTime &time)
{
    settings.setValue("display/redshiftStart", time.toString());
    processTimer();
}

void InfoPaneDropdown::on_endRedshift_timeChanged(const QTime &time)
{
    settings.setValue("display/redshiftEnd", time.toString());
    processTimer();
}

void InfoPaneDropdown::on_redshiftIntensity_sliderMoved(int position)
{
    QProcess::startDetached("redshift -O " + QString::number(position));
}

void InfoPaneDropdown::on_redshiftIntensity_sliderReleased()
{
    if (!isRedshiftOn) {
        QProcess::startDetached("redshift -O 6500");
    }
}

void InfoPaneDropdown::on_redshiftIntensity_valueChanged(int value)
{
    settings.setValue("display/redshiftIntensity", value);
}

void InfoPaneDropdown::newNotificationReceived(int id, QString summary, QString body, QIcon icon) {
    QFrame* frame = new QFrame();
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(0);
    frame->setLayout(layout);

    QLabel* iconLabel = new QLabel();
    iconLabel->setPixmap(icon.pixmap(22, 22));
    layout->addWidget(iconLabel);

    QLabel* sumLabel = new QLabel();
    sumLabel->setText(summary);
    QFont font = sumLabel->font();
    font.setBold(true);
    sumLabel->setFont(font);
    layout->addWidget(sumLabel);

    QLabel* bodyLabel = new QLabel();
    bodyLabel->setText(body);
    bodyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(bodyLabel);

    QLabel* dateLabel = new QLabel();
    dateLabel->setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
    layout->addWidget(dateLabel);

    QPushButton* button = new QPushButton();
    button->setIcon(QIcon::fromTheme("window-close"));
    connect(button, &QPushButton::clicked, [=]() {
        emit closeNotification(id);
    });
    layout->addWidget(button);

    ui->notificationsList->layout()->addWidget(frame);
    ui->noNotifications->setVisible(false);
    ui->clearAllNotifications->setVisible(true);
    notificationFrames.insert(id, frame);

    emit numNotificationsChanged(notificationFrames.count());
}

void InfoPaneDropdown::removeNotification(int id) {
    if (notificationFrames.keys().contains(id)) {
        delete notificationFrames.value(id);
        notificationFrames.remove(id);
    }

    emit numNotificationsChanged(notificationFrames.count());

    if (notificationFrames.count() == 0) {
        ui->noNotifications->setVisible(true);
        ui->clearAllNotifications->setVisible(false);
    }
}

void InfoPaneDropdown::on_clearAllNotifications_clicked()
{
    for (int id : notificationFrames.keys()) {
        emit closeNotification(id);
    }
}


void InfoPaneDropdown::on_redshiftPause_toggled(bool checked)
{
    processTimer();
    settings.setValue("display/redshiftPaused", !checked);
}

void InfoPaneDropdown::batteryLevelChanged(int battery) {
    ui->currentBattery->setText("Current Battery Percentage: " + QString::number(battery));
}

void InfoPaneDropdown::on_printLabel_clicked()
{
    changeDropDown(Print);
}

bool InfoPaneDropdown::isQuietOn() {
    return ui->QuietCheck->isChecked();
}

bool InfoPaneDropdown::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = (QKeyEvent*) event;
        if (keyEvent->key() == Qt::Key_Left) {
            if (ui->pushButton_5->isEnabled()) {
                on_pushButton_5_clicked();
            }
            return true;
        } else if (keyEvent->key() == Qt::Key_Right) {
            if (ui->pushButton_6->isEnabled()) {
                on_pushButton_6_clicked();
            }
            return true;
        }
    }
    return false;
}

void InfoPaneDropdown::on_resetButton_clicked()
{
    if (QMessageBox::warning(this, "Reset theShell",
                             "All settings will be reset to default, and you will be logged out. "
                             "Are you sure you want to do this?", QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) == QMessageBox::Yes) {
        settings.clear();
        EndSession(EndSessionWait::logout);
    }
}

bool InfoPaneDropdown::isTimerRunning() {
    if (timer == NULL) {
        return false;
    } else {
        return true;
    }
}

void InfoPaneDropdown::mousePressEvent(QMouseEvent *event) {
    mouseClickPoint = event->localPos().toPoint().y();
    initialPoint = mouseClickPoint;
    dragRect = this->geometry();
    event->accept();
}

void InfoPaneDropdown::mouseMoveEvent(QMouseEvent *event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    if (event->globalY() < mouseClickPoint) {
        mouseMovedUp = true;
    } else {
        mouseMovedUp = false;
    }

    //dragRect.translate(0, event->localPos().toPoint().y() - mouseClickPoint + this->y());
    dragRect = screenGeometry;
    dragRect.translate(0, event->globalY() - (initialPoint + screenGeometry.top()));

    //innerRect.translate(event->localPos().toPoint().y() - mouseClickPoint, 0);
    if (dragRect.bottom() >= screenGeometry.bottom()) {
        dragRect.moveTo(screenGeometry.left(), screenGeometry.top());
    }
    this->setGeometry(dragRect);

    mouseClickPoint = event->globalY();
    event->accept();
}

void InfoPaneDropdown::mouseReleaseEvent(QMouseEvent *event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (initialPoint - 5 > mouseClickPoint && initialPoint + 5 < mouseClickPoint) {
        QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
        a->setStartValue(this->geometry());
        a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), this->height()));
        a->setEasingCurve(QEasingCurve::OutCubic);
        a->setDuration(500);
        connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
        a->start();
    } else {
        if (mouseMovedUp) {
            this->close();
        } else {
            QPropertyAnimation* a = new QPropertyAnimation(this, "geometry");
            a->setStartValue(this->geometry());
            a->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), this->width(), this->height()));
            a->setEasingCurve(QEasingCurve::OutCubic);
            a->setDuration(500);
            connect(a, SIGNAL(finished()), a, SLOT(deleteLater()));
            a->start();
        }
    }
    event->accept();
    initialPoint = 0;
}

void InfoPaneDropdown::on_TouchFeedbackSwitch_toggled(bool checked)
{
    settings.setValue("input/touchFeedbackSound", checked);
}

void InfoPaneDropdown::on_thewaveTTSpico2wave_clicked()
{
    settings.setValue("thewave/ttsEngine", "pico2wave");
}

void InfoPaneDropdown::on_thewaveTTSfestival_clicked()
{
    settings.setValue("thewave/ttsEngine", "festival");
}


void InfoPaneDropdown::on_thewaveWikipediaSwitch_toggled(bool checked)
{
    settings.setValue("thewave/wikipediaSearch", checked);
}

void InfoPaneDropdown::on_thewaveTTSespeak_clicked()
{
    settings.setValue("thewave/ttsEngine", "espeak");

}

void InfoPaneDropdown::on_thewaveOffensiveSwitch_toggled(bool checked)
{
    settings.setValue("thewave/blockOffensiveWords", checked);
}

void InfoPaneDropdown::on_theWaveName_textEdited(const QString &arg1)
{
    settings.setValue("thewave/name", arg1);
}
