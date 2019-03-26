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
#include "securityinformationwidget.h"
#include "ui_securityinformationwidget.h"

#include <QFileDialog>

SecurityInformationWidget::SecurityInformationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SecurityInformationWidget)
{
    ui->setupUi(this);
}

SecurityInformationWidget::~SecurityInformationWidget()
{
    delete ui;
}

void SecurityInformationWidget::on_SecurityType_currentIndexChanged(int index)
{
    switch (index) {
        case 0: //No security
            ui->SecurityKeysStack->setCurrentIndex(0);
            break;
        case 1: //Static WEP
            ui->SecurityKeysStack->setCurrentIndex(1);
            break;
        case 2: //Dynamic WEP
            ui->SecurityKeysStack->setCurrentIndex(1);
            break;
        case 3: //WPA(2)-PSK
            ui->SecurityKeysStack->setCurrentIndex(1);
            break;
        case 4: //WPA(2) Enterprise
            ui->SecurityKeysStack->setCurrentIndex(2);
            break;
    }
}

void SecurityInformationWidget::on_EnterpriseAuthMethod_currentIndexChanged(int index)
{
    if (index == 4) {
        ui->peapVersionButtons->setVisible(true);
        ui->peapVersionLabel->setVisible(true);
        ui->WpaEnterpriseAuthDetails->setCurrentIndex(3);
    } else {
        ui->peapVersionButtons->setVisible(false);
        ui->peapVersionLabel->setVisible(false);
        ui->WpaEnterpriseAuthDetails->setCurrentIndex(index);
    }
}

QString SecurityInformationWidget::selectCertificate() {
    QFileDialog* dialog = new QFileDialog(this);
    dialog->setNameFilter("Certificates (*.der *.pem *.crt *.cer)");
    if (dialog->exec() == QFileDialog::Accepted) {
        dialog->deleteLater();
        return dialog->selectedFiles().first();
    } else {
        dialog->deleteLater();
        return "";
    }
}

void SecurityInformationWidget::on_EnterpriseTLSUserCertificateSelect_clicked()
{
    ui->EnterpriseTLSUserCertificate->setText(selectCertificate());
}

void SecurityInformationWidget::on_EnterpriseTLSCACertificateSelect_clicked()
{
    ui->EnterpriseTLSCACertificate->setText(selectCertificate());
}

void SecurityInformationWidget::on_EnterprisePEAPCaCertificateSelect_clicked()
{
    ui->EnterprisePEAPCaCertificate->setText(selectCertificate());
}

QVariantMap SecurityInformationWidget::getSecurity() {
    QVariantMap security;
    switch (ui->SecurityType->currentIndex()) {
        case 0: //No security
            security.insert("key-mgmt", "none");
            break;
        case 1: //Static WEP
            security.insert("key-mgmt", "none");
            security.insert("auth-alg", "shared");
            security.insert("wep-key0", ui->securityKey->text());
            break;
        case 2: //Dynamic WEP
            security.insert("key-mgmt", "none");
            security.insert("auth-alg", "shared");
            security.insert("wep-key0", ui->securityKey->text());
            break;
        case 3: //WPA(2)-PSK
            security.insert("key-mgmt", "wpa-psk");
            security.insert("psk", ui->securityKey->text());
            break;
        case 4: { //WPA(2)-Enterprise
            security.insert("key-mgmt", "wpa-eap");
            break;
        }
    }
    return security;
}

QVariantMap SecurityInformationWidget::getEap() {
    QVariantMap enterpriseSettings;
    enterpriseSettings.insert("eap", QStringList() << "ttls");

    switch (ui->EnterpriseAuthMethod->currentIndex()) {
        case 0: //TLS
            enterpriseSettings.insert("eap", QStringList() << "tls");
            enterpriseSettings.insert("identity", ui->EnterpriseTLSIdentity->text());
            enterpriseSettings.insert("client-cert", QUrl::fromLocalFile(ui->EnterpriseTLSUserCertificate->text()).toEncoded());
            enterpriseSettings.insert("ca-cert", QUrl::fromLocalFile(ui->EnterpriseTLSCACertificate->text()).toEncoded());
            enterpriseSettings.insert("subject-match", ui->EnterpriseTLSSubjectMatch->text());
            enterpriseSettings.insert("altsubject-matches", ui->EnterpriseTLSAlternateSubjectMatch->text().split(","));
            enterpriseSettings.insert("private-key", QUrl::fromLocalFile(ui->EnterpriseTLSPrivateKey->text()).toEncoded());
            enterpriseSettings.insert("private-key-password", ui->EnterpriseTLSPrivateKeyPassword->text());
            break;
        case 1: //LEAP
            enterpriseSettings.insert("eap", QStringList() << "leap");
            enterpriseSettings.insert("identity", ui->EnterpriseLEAPUsername->text());
            enterpriseSettings.insert("password", ui->EnterpriseLEAPPassword->text());
            break;
        case 2: { //FAST
            enterpriseSettings.insert("eap", QStringList() << "fast");
            enterpriseSettings.insert("anonymous-identity", ui->EnterpriseFASTAnonymousIdentity->text());
            enterpriseSettings.insert("pac-file", ui->EnterpriseFASTPacFile->text());

            int provisioning = 0;
            if (ui->EnterpriseFASTPacProvisioningAnonymous->isChecked()) provisioning++;
            if (ui->EnterpriseFASTPacProvisioningAuthenticated->isChecked()) provisioning += 2;
            enterpriseSettings.insert("phase1-fast-provisioning", QString::number(provisioning));

            if (ui->EnterpriseFASTPhase2Auth->currentIndex() == 0) { //GTC
                enterpriseSettings.insert("phase2-auth", "gtc");
            } else if (ui->EnterpriseFASTPhase2Auth->currentIndex() == 1) { //MSCHAPv2
                enterpriseSettings.insert("phase2-auth", "mschapv2");
            }

            enterpriseSettings.insert("identity", ui->EnterpriseFASTUsername->text());
            enterpriseSettings.insert("password", ui->EnterpriseFASTPassword->text());

            break;
        }
        case 4: //PEAP
            enterpriseSettings.insert("eap", QStringList() << "peap");

            if (ui->EnterprisePEAPVer0->isChecked()) { //Force version 0
                enterpriseSettings.insert("phase1-peapver", "0");
            } else if (ui->EnterprisePEAPVer1->isChecked()) { //Force version 1
                enterpriseSettings.insert("phase1-peapver", "1");
            }

            //fall through
        case 3: //TTLS
            if (ui->EnterprisePEAPAnonymousIdentity->text() != "") {
                enterpriseSettings.insert("anonymous-identity", ui->EnterprisePEAPAnonymousIdentity->text());
            }

            if (ui->EnterprisePEAPCaCertificate->text() != "") {
                enterpriseSettings.insert("client-cert", QUrl::fromLocalFile(ui->EnterprisePEAPCaCertificate->text()).toEncoded());
            }

            if (ui->EnterprisePEAPPhase2Auth->currentIndex() == 0) { //MSCHAPv2
                enterpriseSettings.insert("phase2-auth", "mschapv2");
            } else if (ui->EnterprisePEAPPhase2Auth->currentIndex() == 1) { //MD5
                enterpriseSettings.insert("phase2-auth", "md5");
            } else if (ui->EnterprisePEAPPhase2Auth->currentIndex() == 2) { //GTC
                enterpriseSettings.insert("phase2-auth", "gtc");
            }

            enterpriseSettings.insert("identity", ui->EnterprisePEAPUsername->text());
            enterpriseSettings.insert("password", ui->EnterprisePEAPPassword->text());
            break;
    }

    return enterpriseSettings;
}

QVariantMap SecurityInformationWidget::getNetwork() {
    QVariantMap wireless;
    wireless.insert("ssid", ui->SecuritySsidEdit->text().toUtf8());
    wireless.insert("mode", "infrastructure");

    if (ui->SecuritySsidEdit->isVisible()) {
        wireless.insert("hidden", true);
    }
    return wireless;
}

void SecurityInformationWidget::addNewNetwork(QString ssid, SecurityType security) {
    ui->SecuritySsidEdit->setText(ssid);
    ui->SecuritySsidEdit->setVisible(false);

    switch (security) {
        case NoSecurity:
            ui->SecurityType->setCurrentIndex(0);
            ui->securityDescriptionLabel->setText(tr("Connect to %1?").arg(ssid));
            break;
        case Leap:
        case StaticWep:
            ui->SecurityType->setCurrentIndex(1);
            ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide a key.").arg(ssid));
            break;
        case DynamicWep:
            ui->SecurityType->setCurrentIndex(2);
            ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide a key.").arg(ssid));
            break;
        case WpaPsk:
        case Wpa2Psk:
            ui->SecurityType->setCurrentIndex(3);
            ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide a key.").arg(ssid));
            break;
        case WpaEnterprise:
        case Wpa2Enterprise:
            ui->SecurityType->setCurrentIndex(4);
            ui->securityDescriptionLabel->setText(tr("To connect to %1, you'll need to provide authentication details.").arg(ssid));
            break;
    }
    ui->SecurityType->setVisible(false);
}

void SecurityInformationWidget::addNewNetwork() {
    ui->SecuritySsidEdit->setVisible(true);
    ui->SecurityType->setVisible(true);
    ui->securityDescriptionLabel->setText(tr("Enter the information to connect to a new network"));
}
