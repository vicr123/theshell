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
#include "settingspane.h"
#include "ui_settingspane.h"

#include "applicationnotificationmodel.h"
#include <QSoundEffect>
#include <QScroller>

struct SettingsPanePrivate {
    QSharedPointer<NotificationsPermissionEngine> currentSettings;
    QSettings settings;
};

SettingsPane::SettingsPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsPane)
{
    ui->setupUi(this);
    d = new SettingsPanePrivate();

    this->settingAttributes.icon = QIcon::fromTheme("preferences-system-notifications");
    this->settingAttributes.menuWidget = ui->LeftPaneWidget;
    this->settingAttributes.providesLeftPane = true;

    ui->LeftPaneWidget->setFixedWidth(300 * theLibsGlobal::getDPIScaling());

    ui->appList->setModel(new ApplicationNotificationModel());
    ui->appList->setItemDelegate(new ApplicationNotificationModelDelegate());
    ui->appList->setIconSize(QSize(32, 32) * theLibsGlobal::getDPIScaling());
    connect(ui->appList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [=](QModelIndex current, QModelIndex previous) {
        if (current.row() == 0) {
            ui->stackedWidget->setCurrentIndex(0);
        } else {
            ui->stackedWidget->setCurrentIndex(1);
            ApplicationInformation info = current.data(Qt::UserRole).value<ApplicationInformation>();
            d->currentSettings = info.permissionsEngine();

            ui->appTitle->setText(info.name);
            ui->allowNotificationsMasterSwitch->setChecked(d->currentSettings->allowNotifications());
            ui->allowSoundsSwitch->setChecked(d->currentSettings->playSound());
            ui->allowPopupsSwitch->setChecked(d->currentSettings->showPopups());
            ui->bypassQuietModeSwitch->setChecked(d->currentSettings->bypassesQuietMode());
        }
    });
    ui->appList->setCurrentIndex(ui->appList->model()->index(0, 0));

    if (d->settings.contains("notifications/lockScreen")) {
        if (d->settings.value("notifications/lockScreen").toString() == "contents") {
            ui->showContentsButton->setChecked(true);
        } else if (d->settings.value("notifications/lockScreen").toString() == "none") {
            ui->dontShowButton->setChecked(true);
        } else {
            ui->hideContentsButton->setChecked(true);
        }
    } else {
        ui->hideContentsButton->setChecked(true);
    }

    ui->notificationSoundBox->blockSignals(true);
    QString notificationSound = d->settings.value("notifications/sound", "tripleping").toString();
    if (notificationSound == "tripleping") {
        ui->notificationSoundBox->setCurrentIndex(0);
    } else if (notificationSound == "upsidedown") {
        ui->notificationSoundBox->setCurrentIndex(1);
    } else if (notificationSound == "echo") {
        ui->notificationSoundBox->setCurrentIndex(2);
    }
    ui->notificationSoundBox->blockSignals(false);

    ui->emphasiseSendingAppSwitch->setChecked(d->settings.value("notifications/emphasiseApp", true).toBool());
    ui->attenuateOnNotification->setChecked(d->settings.value("notifications/attenuate", true).toBool());
    ui->connectMediaSwitch->setChecked(d->settings.value("notifications/mediaInsert", true).toBool());
    ui->chargingSwitch->setChecked(d->settings.value("power/notifyConnectPower", true).toBool());
    ui->unplugSwitch->setChecked(d->settings.value("power/notifyUnplugPower", true).toBool());

    QScroller::grabGesture(ui->appList, QScroller::LeftMouseButtonGesture);
}

SettingsPane::~SettingsPane()
{
    delete d;
    delete ui;
}

QWidget* SettingsPane::mainWidget() {
    return this;
}

QString SettingsPane::name() {
    return tr("Notifications");
}

StatusCenterPaneObject::StatusPaneTypes SettingsPane::type() {
    return Setting;
}

int SettingsPane::position() {
    return 0;
}

void SettingsPane::message(QString name, QVariantList args) {

}


void SettingsPane::on_backButton_clicked()
{
    sendMessage("main-menu", QVariantList());
}

void SettingsPane::on_showContentsButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("notifications/lockScreen", "contents");
    }
}

void SettingsPane::on_hideContentsButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("notifications/lockScreen", "noContents");
    }
}

void SettingsPane::on_dontShowButton_toggled(bool checked)
{
    if (checked) {
        d->settings.setValue("notifications/lockScreen", "none");
    }
}

void SettingsPane::on_emphasiseSendingAppSwitch_toggled(bool checked)
{
    d->settings.setValue("notifications/emphasiseApp", checked);
}

void SettingsPane::on_notificationSoundBox_currentIndexChanged(int index)
{
    QSoundEffect* sound = new QSoundEffect();
    switch (index) {
        case 0:
            d->settings.setValue("notifications/sound", "tripleping");
            sound->setSource(QUrl("qrc:/sounds/notifications/tripleping.wav"));
            break;
        case 1:
            d->settings.setValue("notifications/sound", "upsidedown");
            sound->setSource(QUrl("qrc:/sounds/notifications/upsidedown.wav"));
            break;
        case 2:
            d->settings.setValue("notifications/sound", "echo");
            sound->setSource(QUrl("qrc:/sounds/notifications/echo.wav"));
            break;
    }
    sound->play();
    connect(sound, SIGNAL(playingChanged()), sound, SLOT(deleteLater()));
}

void SettingsPane::on_attenuateOnNotification_toggled(bool checked)
{
    d->settings.setValue("notifications/attenuate", checked);
}

void SettingsPane::on_allowNotificationsMasterSwitch_toggled(bool checked)
{
    d->currentSettings->setAllowNotifications(checked);
    ui->notificationsConfigurationWidget->setEnabled(checked);
}

void SettingsPane::on_allowPopupsSwitch_toggled(bool checked)
{
    d->currentSettings->setShowPopups(checked);
}

void SettingsPane::on_allowSoundsSwitch_toggled(bool checked)
{
    d->currentSettings->setPlaySound(checked);
}

void SettingsPane::on_bypassQuietModeSwitch_toggled(bool checked)
{
    d->currentSettings->setBypassesQuietMode(checked);
}

void SettingsPane::on_connectMediaSwitch_toggled(bool checked)
{
    d->settings.setValue("notifications/mediaInsert", checked);
}

void SettingsPane::on_chargingSwitch_toggled(bool checked)
{
    d->settings.setValue("power/notifyConnectPower", checked);
}

void SettingsPane::on_unplugSwitch_toggled(bool checked)
{
    d->settings.setValue("power/notifyUnplugPower", checked);
}
