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
#include "addlocaledialog.h"
#include "ui_addlocaledialog.h"

#include <locale/localemodel.h>
#include <locale/localemanager.h>
#include <locale/localegroupmodel.h>

struct AddLocaleDialogPrivate {
    LocaleModel* model;
    LocaleGroupModel* lgModel;

    QLocale::Language currentLanguage;
};

AddLocaleDialog::AddLocaleDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddLocaleDialog)
{
    ui->setupUi(this);
    d = new AddLocaleDialogPrivate();

    d->model = new LocaleModel();
    ui->languageList->setModel(d->model);

    d->lgModel = new LocaleGroupModel();
    ui->regionList->setModel(d->lgModel);

    ui->stackedWidget->setCurrentAnimation(tStackedWidget::SlideHorizontal);
}

AddLocaleDialog::~AddLocaleDialog()
{
    delete d;
    delete ui;
}

void AddLocaleDialog::on_languageList_activated(const QModelIndex &index)
{
//    LocaleManager::addLocale(d->model->locale(index));
    QLocale::Language lang = d->model->language(index);
    d->currentLanguage = lang;
    QList<QLocale::Country> countries = QLocale::countriesForLanguage(lang);
    if (countries.count() == 1) {
        LocaleManager::addLocale(QLocale(lang, countries.first()));
        emit done();
    } else {
        ui->regionTitle->setText(QLocale::languageToString(lang));
        d->lgModel->setLanguage(lang);
        ui->stackedWidget->setCurrentWidget(ui->regionPage);
    }
}

void AddLocaleDialog::on_backButton_clicked()
{
    emit done();
}

void AddLocaleDialog::on_backButton_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->mainLanguagePage);
}

void AddLocaleDialog::on_regionList_activated(const QModelIndex &index)
{
    QLocale loc(d->currentLanguage, d->lgModel->country(index));
    LocaleManager::addLocale(loc);
    emit done();
}
