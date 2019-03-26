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
#ifndef SECURITYINFORMATIONWIDGET_H
#define SECURITYINFORMATIONWIDGET_H

#include <QWidget>
#include "availablenetworkslist.h"

namespace Ui {
    class SecurityInformationWidget;
}

class SecurityInformationWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SecurityInformationWidget(QWidget *parent = nullptr);
        ~SecurityInformationWidget();

        void addNewNetwork();
        void addNewNetwork(QString ssid, SecurityType security);

    public slots:
        void on_SecurityType_currentIndexChanged(int index);

        void on_EnterpriseAuthMethod_currentIndexChanged(int index);

        QString selectCertificate();

        void on_EnterpriseTLSUserCertificateSelect_clicked();

        void on_EnterpriseTLSCACertificateSelect_clicked();

        void on_EnterprisePEAPCaCertificateSelect_clicked();

        QVariantMap getSecurity();

        QVariantMap getEap();

        QVariantMap getNetwork();

    private:
        Ui::SecurityInformationWidget *ui;
};

#endif // SECURITYINFORMATIONWIDGET_H
