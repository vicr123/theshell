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
#include "wifisettingspane.h"
#include "ui_wifisettingspane.h"

#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>
#include <NetworkManagerQt/Security8021xSetting>

struct WifiSettingsPanePrivate {
    NetworkManager::WirelessSetting::Ptr settings;
    NetworkManager::WirelessSecuritySetting::Ptr securitySettings;
    NetworkManager::Security8021xSetting::Ptr eapSettings;
};

WifiSettingsPane::WifiSettingsPane(NetworkManager::Connection::Ptr connection, QWidget *parent) :
    SettingPane(connection, parent),
    ui(new Ui::WifiSettingsPane)
{
    ui->setupUi(this);
    d = new WifiSettingsPanePrivate();
}

WifiSettingsPane::~WifiSettingsPane()
{
    delete ui;
    delete d;
}

void WifiSettingsPane::updateFields()
{
    d->settings = connection->settings()->setting(NetworkManager::Setting::Wireless).staticCast<NetworkManager::WirelessSetting>();

    ui->ssidField->setText(d->settings->ssid());
    ui->modeComboBox->setCurrentIndex(d->settings->mode());
    ui->mtuBox->setValue(static_cast<int>(d->settings->mtu()));
    ui->hiddenNetworkSwitch->setChecked(d->settings->hidden());

    bool loadEapSettings = false;
    d->securitySettings = connection->settings()->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
    switch (d->securitySettings->authAlg()) {
        case NetworkManager::WirelessSecuritySetting::None:
            ui->securityComboBox->setCurrentIndex(0);
            break;
        default:
            switch (d->securitySettings->keyMgmt()) {
                case NetworkManager::WirelessSecuritySetting::WpaEap:
                    ui->securityComboBox->setCurrentIndex(2);
                    loadEapSettings = true;
                    break;
                case NetworkManager::WirelessSecuritySetting::WpaPsk:
                    ui->securityComboBox->setCurrentIndex(3);
                    break;
                case NetworkManager::WirelessSecuritySetting::WpaNone:
                default:
                    ui->securityComboBox->setCurrentIndex(1);
            }
    }

    d->eapSettings = connection->settings()->setting(NetworkManager::Setting::Security8021x).staticCast<NetworkManager::Security8021xSetting>();

    if (loadEapSettings) {
        switch (d->eapSettings->eapMethods().first()) {
            case NetworkManager::Security8021xSetting::EapMethodTls:
                ui->EnterpriseAuthMethod->setCurrentIndex(0);
                break;
            case NetworkManager::Security8021xSetting::EapMethodLeap:
                ui->EnterpriseAuthMethod->setCurrentIndex(1);
                break;
            case NetworkManager::Security8021xSetting::EapMethodFast:
                ui->EnterpriseAuthMethod->setCurrentIndex(2);
                break;
            case NetworkManager::Security8021xSetting::EapMethodTtls:
                ui->EnterpriseAuthMethod->setCurrentIndex(3);
                break;
            case NetworkManager::Security8021xSetting::EapMethodPeap:
                ui->EnterpriseAuthMethod->setCurrentIndex(4);
                break;
            default:
                break;
        }
    }
}

void WifiSettingsPane::on_ssidField_textEdited(const QString &arg1)
{
    d->settings->setSsid(arg1.toUtf8());
    settingsChanged();
}

void WifiSettingsPane::on_modeComboBox_currentIndexChanged(int index)
{
    d->settings->setMode(static_cast<NetworkManager::WirelessSetting::NetworkMode>(index));
    settingsChanged();
}

void WifiSettingsPane::on_mtuBox_valueChanged(int arg1)
{
    d->settings->setMtu(static_cast<quint32>(arg1));
    settingsChanged();
}

void WifiSettingsPane::on_hiddenNetworkSwitch_toggled(bool checked)
{
    d->settings->setHidden(checked);
}

void WifiSettingsPane::on_securityComboBox_currentIndexChanged(int index)
{
    switch (index) {
        case 0: //No security
            ui->SecurityKeysStack->setCurrentIndex(0);
            d->securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
            d->securitySettings->setAuthAlg(NetworkManager::WirelessSecuritySetting::None);
            break;
        case 1: //Static WEP
        case 2: //Dynamic WEP
            ui->SecurityKeysStack->setCurrentIndex(1);
            d->securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaNone);
            d->securitySettings->setAuthAlg(NetworkManager::WirelessSecuritySetting::Shared);
            break;
        case 3: //WPA(2)-PSK
            ui->SecurityKeysStack->setCurrentIndex(1);
            d->securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
            d->securitySettings->setAuthAlg(NetworkManager::WirelessSecuritySetting::Shared);
            break;
        case 4: //WPA(2) Enterprise
            ui->SecurityKeysStack->setCurrentIndex(2);
            d->securitySettings->setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
            d->securitySettings->setAuthAlg(NetworkManager::WirelessSecuritySetting::Shared);
            break;
    }

    settingsChanged();
}

void WifiSettingsPane::on_EnterpriseAuthMethod_currentIndexChanged(int index)
{

    switch (ui->EnterpriseAuthMethod->currentIndex()) {
        case 0: //TLS
            d->eapSettings->setEapMethods({NetworkManager::Security8021xSetting::EapMethodTls});
            break;
        case 1: //LEAP
            d->eapSettings->setEapMethods({NetworkManager::Security8021xSetting::EapMethodLeap});
            break;
        case 2: //FAST
            d->eapSettings->setEapMethods({NetworkManager::Security8021xSetting::EapMethodFast});
            break;
        case 3: //TTLS
            d->eapSettings->setEapMethods({NetworkManager::Security8021xSetting::EapMethodTtls});
            break;
        case 4: //PEAP
            d->eapSettings->setEapMethods({NetworkManager::Security8021xSetting::EapMethodPeap});
    }

    if (index == 4) {
        ui->peapVersionButtons->setVisible(true);
        ui->peapVersionLabel->setVisible(true);
        ui->WpaEnterpriseAuthDetails->setCurrentIndex(3);
    } else {
        ui->peapVersionButtons->setVisible(false);
        ui->peapVersionLabel->setVisible(false);
        ui->WpaEnterpriseAuthDetails->setCurrentIndex(index);
    }

    settingsChanged();
}
