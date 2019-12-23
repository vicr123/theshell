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
#include "localepane.h"
#include "ui_localepane.h"

#include <locale/localemanager.h>
#include <locale/currentlocalesmodel.h>
#include "addlocaledialog.h"
#include <tpopover.h>
#include <QMenu>

struct LocalePanePrivate {
    CurrentLocalesModel* currentLocalesModel;
};

LocalePane::LocalePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalePane)
{
    ui->setupUi(this);

    d = new LocalePanePrivate();
    d->currentLocalesModel = new CurrentLocalesModel();

    ui->activeLanguagesList->setModel(d->currentLocalesModel);

    this->settingAttributes.icon = QIcon::fromTheme("preferences-system-locale");
}

LocalePane::~LocalePane()
{
    delete d;
    delete ui;
}


QWidget*LocalePane::mainWidget()
{
    return this;
}

QString LocalePane::name()
{
    return tr("Language and Region");
}

StatusCenterPaneObject::StatusPaneTypes LocalePane::type()
{
    return Setting;
}

int LocalePane::position()
{
    return 800;
}

void LocalePane::message(QString name, QVariantList args)
{

}

void LocalePane::on_activeLanguagesList_activated(const QModelIndex &index)
{
    if (index.row() == d->currentLocalesModel->rowCount() - 1) {
        AddLocaleDialog* d = new AddLocaleDialog();
        tPopover* popover = new tPopover(d);
        popover->setPopoverWidth(SC_DPI(600));
        connect(d, &AddLocaleDialog::done, popover, &tPopover::dismiss);
        connect(popover, &tPopover::dismissed, d, &AddLocaleDialog::deleteLater);
        connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
        popover->show(this->window());
    }
}

void LocalePane::on_activeLanguagesList_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    QLocale currentLocale = d->currentLocalesModel->locale(ui->activeLanguagesList->currentIndex());
    bool moveUp = true;
    bool moveDown = true;
    bool canRemove = true;
    if (d->currentLocalesModel->rowCount() == 2) canRemove = false;
    if (ui->activeLanguagesList->currentIndex().row() == 0) moveUp = false;
    if (ui->activeLanguagesList->currentIndex().row() == d->currentLocalesModel->rowCount() - 2) moveDown = false;

    menu.addSection(tr("For this language"));
    if (moveUp) menu.addAction(QIcon::fromTheme("go-up"), tr("Move Up"), this, [=] {
        LocaleManager::moveLocaleUp(currentLocale);
    });
    if (moveDown) menu.addAction(QIcon::fromTheme("go-down"), tr("Move Down"), this, [=] {
        LocaleManager::moveLocaleDown(currentLocale);
    });
    if (canRemove && (moveUp || moveDown)) menu.addSeparator();
    if (canRemove) menu.addAction(QIcon::fromTheme("list-remove"), tr("Remove"), this, [=] {
        LocaleManager::removeLocale(currentLocale);
    });
    menu.exec(ui->activeLanguagesList->mapToGlobal(pos));
}

void LocalePane::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
}
