#include "kdeconnectwidget.h"
#include "ui_kdeconnectwidget.h"

KdeConnectWidget::KdeConnectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KdeConnectWidget)
{
    ui->setupUi(this);

    watcher = new QDBusServiceWatcher("org.kde.kdeconnect", QDBusConnection::sessionBus());
    connect(watcher, SIGNAL(serviceRegistered(QString)), this, SLOT(kdeConnectOnline()));
    connect(watcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(kdeConnectGone()));

    daemon = new QDBusInterface("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon");
    QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect", "org.kde.kdeconnect.daemon", "announcedNameChanged", this, SLOT(kdeConnectAnnouncedNameChanged(QString)));

    devicesModel = new KdeConnectDevicesModel();
    ui->devicesView->setModel(devicesModel);
    ui->devicesView->setItemDelegate(new KdeConnectDevicesDelegate());

    connect(ui->devicesView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(selectedDeviceChanged(QModelIndex,QModelIndex)));

    if (QDBusConnection::sessionBus().interface()->registeredServiceNames().value().contains("org.kde.kdeconnect")) {
        kdeConnectOnline();
    } else {
        kdeConnectGone();
    }
}

KdeConnectWidget::~KdeConnectWidget()
{
    delete ui;
}

void KdeConnectWidget::kdeConnectAnnouncedNameChanged(QString name) {
    ui->announcedName->setText(tr("This device is called <b>%1</b>").arg(name));
}

void KdeConnectWidget::kdeConnectOnline() {
    devicesModel->updateData();
    kdeConnectAnnouncedNameChanged(daemon->call("announcedName").arguments().first().toString());
    ui->panes->setCurrentIndex(0);
}

void KdeConnectWidget::kdeConnectGone() {
    devicesModel->updateData();
    ui->announcedName->setText(tr("KDE Connect is not running"));
    ui->panes->setCurrentIndex(5);
    ui->devicesView->clearSelection();
}

void KdeConnectWidget::selectedDeviceChanged(QModelIndex current, QModelIndex previous) {
    Q_UNUSED(previous)

    if (currentId != "") {
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "reachableChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "pluginsChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "trustedChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "nameChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().disconnect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "hasPairingRequestsChanged", this, SLOT(updateCurrentDevice()));
    }

    if (current.isValid()) {
        ui->panes->setCurrentIndex(1);
        currentId = current.data(Qt::UserRole + 1).toString();

        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "reachableChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "pluginsChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "trustedChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "nameChanged", this, SLOT(updateCurrentDevice()));
        QDBusConnection::sessionBus().connect("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device", "hasPairingRequestsChanged", this, SLOT(updateCurrentDevice()));

        updateCurrentDevice();
    } else {
        currentId = "";
    }
}

void KdeConnectWidget::updateCurrentDevice() {
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");

    ui->deviceName->setText(interface.property("name").toString());

    bool reachable = interface.property("isReachable").toBool();
    bool trusted = interface.property("isTrusted").toBool();
    if (trusted) {
        ui->pairButton->setIcon(QIcon::fromTheme("remove-link"));
        ui->pairButton->setToolTip(tr("Unpair Device"));
    } else {
        ui->pairButton->setIcon(QIcon::fromTheme("insert-link"));
        ui->pairButton->setToolTip(tr("Pair Device"));
    }

    if (reachable && trusted) {
        ui->deviceActionsPane->setVisible(true);
        ui->deviceUnreachablePane->setVisible(false);
        ui->deviceUnpairedPane->setVisible(false);

        QStringList availablePlugins = interface.call("loadedPlugins").arguments().first().toStringList();
        ui->pingButton->setVisible(availablePlugins.contains("kdeconnect_ping"));
        ui->locateButton->setVisible(availablePlugins.contains("kdeconnect_findmyphone"));
        ui->sendFileButton->setVisible(availablePlugins.contains("kdeconnect_share"));
        ui->smsButton->setVisible(availablePlugins.contains("kdeconnect_telephony"));
    } else if (!trusted) {
        ui->deviceActionsPane->setVisible(false);
        ui->deviceUnreachablePane->setVisible(false);
        ui->deviceUnpairedPane->setVisible(true);
    } else {
        ui->deviceActionsPane->setVisible(false);
        ui->deviceUnreachablePane->setVisible(true);
        ui->deviceUnpairedPane->setVisible(false);
    }
}

void KdeConnectWidget::on_pingButton_clicked()
{
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(QDBusMessage::createMethodCall("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId + "/ping", "org.kde.kdeconnect.device.ping", "sendPing")));
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        watcher->deleteLater();

        tToast* t = new tToast();
        t->setTitle(tr("Ping"));
        t->setText(tr("Ping sent to %1").arg(ui->deviceName->text()));
        t->show(this);
        connect(t, SIGNAL(dismissed()), t, SLOT(deleteLater()));
    });
}

void KdeConnectWidget::on_locateButton_clicked()
{
    if (QMessageBox::question(this, tr("Locate Device"), tr("Your device will play its ringtone at full volume. To stop it ringing, tap the button on the device's screen."), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Ok) == QMessageBox::Ok) {
        QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(QDBusMessage::createMethodCall("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId + "/findmyphone", "org.kde.kdeconnect.device.findmyphone", "ring")));
        connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
            watcher->deleteLater();

            tToast* t = new tToast();
            t->setTitle(tr("Locate"));
            t->setText(tr("The request to play a sound has been sent to %1").arg(ui->deviceName->text()));
            t->show(this);
            connect(t, SIGNAL(dismissed()), t, SLOT(deleteLater()));
        });
    }
}

void KdeConnectWidget::on_pairButton_clicked()
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    if (interface.property("isTrusted").toBool()) {
        if (QMessageBox::question(this, tr("Unpair Device"), tr("Do you want to unpair this device?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
            QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(interface.asyncCall("unpair"));
            connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
                watcher->deleteLater();

                tToast* t = new tToast();
                t->setTitle(tr("Unpair"));
                t->setText(tr("%1 has been unpaired from this device. To use KDE Connect between these devices again, you'll need to pair them.").arg(ui->deviceName->text()));
                t->show(this);
                connect(t, SIGNAL(dismissed()), t, SLOT(deleteLater()));
            });
        }
    } else {
        QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(interface.asyncCall("requestPair"));
        connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
            watcher->deleteLater();

            tToast* t = new tToast();
            t->setTitle(tr("Pair"));
            t->setText(tr("A pairing request has been sent to %1. To complete pairing, you'll need to respond on %1.").arg(ui->deviceName->text()));
            t->show(this);
            connect(t, SIGNAL(dismissed()), t, SLOT(deleteLater()));
        });
    }
}

void KdeConnectWidget::on_sendFileButton_clicked()
{
    QFileDialog d;
    d.setWindowTitle("Send File");
    d.setAcceptMode(QFileDialog::AcceptOpen);
    d.setFileMode(QFileDialog::ExistingFile);

    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId + "/share", "org.kde.kdeconnect.device.share");
    if (d.exec() == QFileDialog::Accepted) {
        for (QUrl url : d.selectedUrls()) {
            interface.asyncCall("shareUrl", url.toString(QUrl::FullyDecoded));
        }
    }
}

void KdeConnectWidget::on_settingsButton_clicked()
{
    ui->panes->setCurrentIndex(3);

    //Update plugin settings
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");

    QStringList supportedPlugins = interface.property("supportedPlugins").toStringList();
    ui->pluginBatteryMonitor->setVisible(supportedPlugins.contains("kdeconnect_battery"));
    ui->pluginClipboard->setVisible(supportedPlugins.contains("kdeconnect_clipboard"));
    ui->pluginInhibitScreensaver->setVisible(supportedPlugins.contains("kdeconnect_screensaver_inhibit"));
    ui->pluginMultimediaControlReceiver->setVisible(supportedPlugins.contains("kdeconnect_mpriscontrol"));
    ui->pluginPauseMedia->setVisible(supportedPlugins.contains("kdeconnect_pausemusic"));
    ui->pluginPing->setVisible(supportedPlugins.contains("kdeconnect_ping"));
    ui->pluginNotifications->setVisible(supportedPlugins.contains("kdeconnect_notifications"));
    ui->pluginBrowseFilesystem->setVisible(supportedPlugins.contains("kdeconnect_sftp"));
    ui->pluginRemoteKeyboard->setVisible(supportedPlugins.contains("kdeconnect_remotekeyboard"));
    ui->pluginRingPhone->setVisible(supportedPlugins.contains("kdeconnect_findmyphone"));
    ui->pluginRunCommands->setVisible(supportedPlugins.contains("kdeconnect_runcommand"));
    ui->pluginSendNotifications->setVisible(supportedPlugins.contains("kdeconnect_sendnotifications"));
    ui->pluginSendReceive->setVisible(supportedPlugins.contains("kdeconnect_share"));
    ui->pluginTelephony->setVisible(supportedPlugins.contains("kdeconnect_telephony"));
    ui->pluginVirtualInput->setVisible(supportedPlugins.contains("kdeconnect_mousepad"));

    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(interface.asyncCall("loadedPlugins"));
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        QStringList availablePlugins = watcher->reply().arguments().first().toStringList();
        ui->pluginBatteryMonitor->setChecked(availablePlugins.contains("kdeconnect_battery"));
        ui->pluginClipboard->setChecked(availablePlugins.contains("kdeconnect_clipboard"));
        ui->pluginInhibitScreensaver->setChecked(availablePlugins.contains("kdeconnect_screensaver_inhibit"));
        ui->pluginMultimediaControlReceiver->setChecked(availablePlugins.contains("kdeconnect_mpriscontrol"));
        ui->pluginPauseMedia->setChecked(availablePlugins.contains("kdeconnect_pausemusic"));
        ui->pluginPing->setChecked(availablePlugins.contains("kdeconnect_ping"));
        ui->pluginNotifications->setChecked(availablePlugins.contains("kdeconnect_notifications"));
        ui->pluginBrowseFilesystem->setChecked(availablePlugins.contains("kdeconnect_sftp"));
        ui->pluginRemoteKeyboard->setChecked(availablePlugins.contains("kdeconnect_remotekeyboard"));
        ui->pluginRingPhone->setChecked(availablePlugins.contains("kdeconnect_findmyphone"));
        ui->pluginRunCommands->setChecked(availablePlugins.contains("kdeconnect_runcommand"));
        ui->pluginSendNotifications->setChecked(availablePlugins.contains("kdeconnect_sendnotifications"));
        ui->pluginSendReceive->setChecked(availablePlugins.contains("kdeconnect_share"));
        ui->pluginTelephony->setChecked(availablePlugins.contains("kdeconnect_telephony"));
        ui->pluginVirtualInput->setChecked(availablePlugins.contains("kdeconnect_mousepad"));

        ui->panes->setCurrentIndex(2);

        watcher->deleteLater();
    });
}

void KdeConnectWidget::on_backButton_clicked()
{
    ui->panes->setCurrentIndex(1);
}

void KdeConnectWidget::on_pluginBatteryMonitor_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_battery", checked);
}

void KdeConnectWidget::on_pluginClipboard_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_clipboard", checked);
}

void KdeConnectWidget::on_pluginInhibitScreensaver_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_screensaver_inhibit", checked);
}

void KdeConnectWidget::on_pluginMultimediaControlReceiver_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_mprisControl", checked);
}

void KdeConnectWidget::on_pluginPauseMedia_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_pauseMusic", checked);
}

void KdeConnectWidget::on_pluginPing_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_ping", checked);
}

void KdeConnectWidget::on_pluginNotifications_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_notifications", checked);
}

void KdeConnectWidget::on_pluginBrowseFilesystem_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_sftp", checked);
}

void KdeConnectWidget::on_pluginRemoteKeyboard_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_remotekeyboard", checked);
}

void KdeConnectWidget::on_pluginRingPhone_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_findmyphone", checked);
}

void KdeConnectWidget::on_pluginRunCommands_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_runcommand", checked);
}

void KdeConnectWidget::on_pluginSendNotifications_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_sendnotifications", checked);
}

void KdeConnectWidget::on_pluginSendReceive_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_share", checked);
}

void KdeConnectWidget::on_pluginTelephony_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_telephony", checked);
}

void KdeConnectWidget::on_pluginVirtualInput_toggled(bool checked)
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    interface.asyncCall("setPluginEnabled", "kdeconnect_mousepad", checked);
}

void KdeConnectWidget::on_smsButton_clicked()
{
    ui->panes->setCurrentIndex(4);
}

void KdeConnectWidget::on_smsMessage_textChanged()
{
    QStringList remaining;
    int numLeft = ui->smsMessage->toPlainText().length() % 160;
    int numMessages = (ui->smsMessage->toPlainText().length() / 160) + 1;

    if (numLeft == 0) {
        numLeft = 160;
        numMessages--;
    }

    if (numMessages > 1) {
        remaining += tr("Sending as %n messages", nullptr, numMessages);
    }
    remaining += QString::number(numLeft) + "/160";
    ui->smsCounter->setText(remaining.join(" · "));
}

void KdeConnectWidget::on_backButton_3_clicked()
{
    ui->panes->setCurrentIndex(1);
}

void KdeConnectWidget::on_sendSMSButton_clicked()
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId + "/telephony", "org.kde.kdeconnect.device.telephony");
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(interface.asyncCall("sendSms", ui->smsNumber->text(), ui->smsMessage->toPlainText()));
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        watcher->deleteLater();

        tToast* t = new tToast();
        t->setTitle(tr("Send SMS"));
        t->setText(tr("Your SMS has been sent to your device and is on its way!").arg(ui->deviceName->text()));
        t->show(this);
        connect(t, SIGNAL(dismissed()), t, SLOT(deleteLater()));
    });

    ui->smsNumber->setText("");
    ui->smsMessage->clear();
    ui->panes->setCurrentIndex(1);
}

void KdeConnectWidget::on_startkdeConnectButton_clicked()
{
    QProcess::startDetached("/usr/lib/kdeconnectd");
}

void KdeConnectWidget::on_encryptionButton_clicked()
{
    QDBusInterface interface("org.kde.kdeconnect", "/modules/kdeconnect/devices/" + currentId, "org.kde.kdeconnect.device");
    QMessageBox::information(this, tr("Encryption Information"), interface.call("encryptionInfo").arguments().first().toString(), QMessageBox::Ok, QMessageBox::Ok);
}

void KdeConnectWidget::on_smsNumber_textEdited(const QString &arg1)
{
    //Remove invalid characters
    QString s = arg1;
    for (int i = 0; i < s.length(); i++) {
        QChar c = s.at(i);
        if (!c.isDigit() && c != '+' && c != ' ') {
            s.remove(i, 1);
            i--;
        }
    }
    ui->smsNumber->setText(s);
}

void KdeConnectWidget::on_renameButton_clicked()
{
    bool ok;
    QString oldName = daemon->call("announcedName").arguments().first().toString();
    QString newName = QInputDialog::getText(this, tr("Rename Device"), tr("What do you want to call this device?"), QLineEdit::Normal, oldName, &ok);
    if (ok) {
        daemon->asyncCall("setAnnouncedName", newName);
    }
}

void KdeConnectWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
