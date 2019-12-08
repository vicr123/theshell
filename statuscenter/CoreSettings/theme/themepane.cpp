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
#include "themepane.h"
#include "ui_themepane.h"

#include <tpopover.h>
#include "colourspane.h"
#include "widgetstylepane.h"

struct ThemePanePrivate {
    QSettings* themeSettings;
};

ThemePane::ThemePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ThemePane)
{
    ui->setupUi(this);
    d = new ThemePanePrivate();

    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-theme");

    d->themeSettings = new QSettings("theSuite", "ts-qtplatform", parent);

    ui->qtWidgetStyleButton->setDescription(d->themeSettings->value("style/name", "Contemporary").toString());
    updateSystemColoursButton();
}

ThemePane::~ThemePane()
{
    delete d;
    delete ui;
}


QWidget*ThemePane::mainWidget()
{
    return this;
}

QString ThemePane::name()
{
    return tr("Theme");
}

StatusCenterPaneObject::StatusPaneTypes ThemePane::type()
{
    return StatusCenterPaneObject::Setting;
}

int ThemePane::position()
{
    return 800;
}

void ThemePane::message(QString name, QVariantList args)
{

}

void ThemePane::on_coloursButton_clicked()
{
    ColoursPane* pane = new ColoursPane();
    tPopover* popover = new tPopover(pane);
    popover->setPopoverSide(tPopover::Bottom);
    popover->setPopoverWidth(pane->sizeHint().height());
    connect(pane, &ColoursPane::reject, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, pane, &ColoursPane::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(popover, &tPopover::dismissed, this, &ThemePane::updateSystemColoursButton);
    popover->show(this->window());
}

void ThemePane::updateSystemColoursButton()
{
    QString themeName;
    QString themeType = d->themeSettings->value("color/type", "dark").toString();
    if (themeType == "light") {
        themeName = tr("Light");
    } else if (themeType == "dark") {
        themeName = tr("Dark");
    } else if (themeType == "black") {
        themeName = tr("Black");
    } else if (themeType == "gray") {
        themeName = tr("Gray");
    } else if (themeType == "decorative") {
        themeName = tr("Decorative");
    }

    QString accentNames[] = {
        tr("blue"),
        tr("green"),
        tr("orange"),
        tr("pink"),
        tr("turquoise")
    };

    int accentIndex = d->themeSettings->value("color/accent", 0).toInt();

    ui->coloursButton->setDescription(tr("%1 theme - %2 accents").arg(themeName).arg(accentNames[accentIndex]));
}

void ThemePane::on_qtWidgetStyleButton_clicked()
{
    WidgetStylePane* pane = new WidgetStylePane();
    tPopover* popover = new tPopover(pane);
    popover->setPopoverSide(tPopover::Bottom);
    popover->setPopoverWidth(pane->sizeHint().height());
    connect(pane, &WidgetStylePane::reject, popover, &tPopover::dismiss);
    connect(pane, &WidgetStylePane::accept, popover, &tPopover::dismiss);
    connect(popover, &tPopover::dismissed, pane, &ColoursPane::deleteLater);
    connect(popover, &tPopover::dismissed, popover, &tPopover::deleteLater);
    connect(pane, &WidgetStylePane::accept, this, [=] {
        ui->qtWidgetStyleButton->setDescription(d->themeSettings->value("style/name", "Contemporary").toString());
        sendMessage("show-restart-required", {});
    });
    popover->show(this->window());
}

void ThemePane::on_iconsButton_clicked()
{

}
