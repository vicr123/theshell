#include "infopanedropdown.h"
#include "ui_infopanedropdown.h"

extern void playSound(QUrl, bool = false);

InfoPaneDropdown::InfoPaneDropdown(NotificationDBus* notificationEngine, UPowerDBus* powerEngine, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoPaneDropdown)
{
    ui->setupUi(this);

    this->notificationEngine = notificationEngine;
    this->powerEngine = powerEngine;

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

    ui->lineEdit_2->setText(settings.value("startup/autostart", "").toString());
    ui->redshiftPause->setChecked(!settings.value("display/redshiftPaused", true).toBool());

    eventTimer = new QTimer(this);
    eventTimer->setInterval(1000);
    connect(eventTimer, SIGNAL(timeout()), this, SLOT(processTimer()));
    eventTimer->start();
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

    switch (changeTo) {
    case Settings:
        ui->settingsFrame->setVisible(true);

        ui->clockLabel->setShowDisabled(true);
        ui->batteryLabel->setShowDisabled(true);
        ui->notificationsLabel->setShowDisabled(true);
        ui->networkLabel->setShowDisabled(true);

        break;
    case Clock:
        ui->clockFrame->setVisible(true);

        ui->clockLabel->setShowDisabled(false);
        ui->batteryLabel->setShowDisabled(true);
        ui->notificationsLabel->setShowDisabled(true);
        ui->networkLabel->setShowDisabled(true);
        break;
    case Battery:
        ui->batteryFrame->setVisible(true);

        ui->clockLabel->setShowDisabled(true);
        ui->batteryLabel->setShowDisabled(false);
        ui->notificationsLabel->setShowDisabled(true);
        ui->networkLabel->setShowDisabled(true);
        break;
    case Notifications:
        ui->notificationsFrame->setVisible(true);

        ui->clockLabel->setShowDisabled(true);
        ui->batteryLabel->setShowDisabled(true);
        ui->notificationsLabel->setShowDisabled(false);
        ui->networkLabel->setShowDisabled(true);
        break;
    case Network:
        ui->networkFrame->setVisible(true);

        ui->clockLabel->setShowDisabled(true);
        ui->batteryLabel->setShowDisabled(true);
        ui->notificationsLabel->setShowDisabled(true);
        ui->networkLabel->setShowDisabled(false);

        getNetworks();
    }

    if (changeTo == 0 || changeTo == -1) {
        ui->pushButton_5->setEnabled(false);
        ui->pushButton_6->setEnabled(true);
    } else if (changeTo == 3) {
        ui->pushButton_5->setEnabled(true);
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
    if (timer != NULL) {
        timer->stop();
        delete timer;
        timer = NULL;
        ui->timeEdit->setVisible(true);
        ui->label_7->setVisible(false);
        ui->label_7->setEnabled(true);
        ui->pushButton_2->setText("Start");
        ui->pushButton_3->setVisible(false);
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
        } else {
            ui->label_7->setText(timeUntilTimeout.toString("HH:mm:ss"));
        }
    });
    timer->start();
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
        } else {
            timer->start();
            ui->pushButton_3->setVisible(false);
            ui->label_7->setEnabled(true);
            ui->pushButton_2->setText("Pause");
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
