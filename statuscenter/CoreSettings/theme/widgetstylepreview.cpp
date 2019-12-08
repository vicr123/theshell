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
#include "widgetstylepreview.h"
#include "ui_widgetstylepreview.h"

#include <QSettings>
#include <QStyleFactory>

WidgetStylePreview::WidgetStylePreview(QString styleKey, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetStylePreview)
{
    ui->setupUi(this);

    iterateChildren(QStyleFactory::create(styleKey), ui->previewFrame);

    ui->styleNameLabel->setText(styleKey.toUpper());
    ui->previewFrame->setVisible(false);
    this->styleName = styleKey;
}

WidgetStylePreview::~WidgetStylePreview()
{
    delete ui;
}

void WidgetStylePreview::iterateChildren(QStyle*style, QWidget*w)
{
    w->setStyle(style);
    for (QObject* child : w->children()) {
        QWidget* w = qobject_cast<QWidget*>(child);
        if (w) iterateChildren(style, w);
    }
}

void WidgetStylePreview::on_selectButton_clicked()
{
    QSettings settings("theSuite", "ts-qtplatform");
    settings.setValue("style/name", this->styleName);
    emit done();
}

void WidgetStylePreview::on_previewButton_toggled(bool checked)
{
    ui->previewFrame->setVisible(checked);
}
