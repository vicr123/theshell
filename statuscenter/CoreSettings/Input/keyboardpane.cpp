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
#include "keyboardpane.h"
#include "ui_keyboardpane.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMenu>
#include <QScroller>
#include <tpromise.h>

struct KeyboardPanePrivate {
    QSettings settings;
};

KeyboardPane::KeyboardPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::KeyboardPane)
{
    ui->setupUi(this);
    d = new KeyboardPanePrivate();

    //Update keyboard layouts
    struct KeyboardLayoutReturn {
        QMap<QString, QString> availableKeyboardLayout;
        QStringList currentKeyboardLayout;
    };

    (new tPromise<KeyboardLayoutReturn>([=](QString& error) {
        KeyboardLayoutReturn retval;

        QDir xkbLayouts("/usr/share/X11/xkb/symbols");
        for (QFileInfo layoutInfo : xkbLayouts.entryInfoList()) {
            if (layoutInfo.isDir()) continue;

            QString layout = layoutInfo.baseName();
            QFile file(layoutInfo.filePath());
            file.open(QFile::ReadOnly);

            QString currentSubLayout = "";
            while (!file.atEnd()) {
                QString line = file.readLine().trimmed();
                if (line.startsWith("xkb_symbols") && line.endsWith("{")) {
                    QRegExp lineRx("\".+\"");
                    lineRx.indexIn(line);

                    if (lineRx.capturedTexts().count() != 0) {
                        currentSubLayout = lineRx.capturedTexts().first().remove("\"");
                    } else {
                        currentSubLayout = "";
                    }
                } else if (line.startsWith("name")) {
                    QRegExp lineRx("\".+\"");
                    lineRx.indexIn(line);

                    if (lineRx.capturedTexts().count() != 0 && currentSubLayout != "") {
                        retval.availableKeyboardLayout.insert(layout + "(" + currentSubLayout + ")", lineRx.capturedTexts().first().remove("\""));
                    } else {
                        currentSubLayout = "";
                    }
                }
            }

            file.close();
        }

        retval.currentKeyboardLayout = d->settings.value("input/layout", "us(basic)").toString().split(",");
        return retval;
    }))->then([=](KeyboardLayoutReturn layouts) {
        for (QString key : layouts.availableKeyboardLayout.keys()) {
            QString value = layouts.availableKeyboardLayout.value(key);

            QListWidgetItem* item = new QListWidgetItem();
            item->setText(value);
            item->setData(Qt::UserRole, key);
            ui->availableKeyboardLayouts->addItem(item);
        }

        for (QString layout : layouts.currentKeyboardLayout) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(layouts.availableKeyboardLayout.value(layout, layout));
            item->setData(Qt::UserRole, layout);
            ui->selectedLayouts->addItem(item);
        }

        QListWidgetItem* item = new QListWidgetItem();
        item->setIcon(QIcon::fromTheme("list-add"));
        item->setText(tr("New Layout"));
        item->setData(Qt::UserRole, "add");
        ui->selectedLayouts->addItem(item);

        ui->availableKeyboardLayouts->sortItems();
        connect(ui->selectedLayouts->model(), SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)), this, SLOT(KeyboardLayoutsMoved()));
    });

    //Load settings
    ui->SuperKeyOpenGatewaySwitch->setChecked(d->settings.value("input/superkeyGateway", true).toBool());

    ui->mainStack->setCurrentAnimation(tStackedWidget::SlideHorizontal);
    QScroller::grabGesture(ui->availableKeyboardLayouts, QScroller::LeftMouseButtonGesture);
}

KeyboardPane::~KeyboardPane()
{
    delete ui;
    delete d;
}


void KeyboardPane::KeyboardLayoutsMoved()
{
    //Completely recreate the selected layouts
    QStringList currentLayouts;
    for (int i = 0; i < ui->selectedLayouts->count(); i++) {
        QListWidgetItem* item = ui->selectedLayouts->item(i);
        if (item->data(Qt::UserRole).toString() != "add") {
            currentLayouts.append(item->data(Qt::UserRole).toString());
        }
    }
    d->settings.setValue("input/layout", currentLayouts.join(","));
    emit loadNewKeyboardLayoutMenu();
}

void KeyboardPane::on_selectedLayouts_itemActivated(QListWidgetItem *item)
{
    if (item->data(Qt::UserRole).toString() == "add") {
        ui->mainStack->setCurrentIndex(1);
    }
}

void KeyboardPane::on_backButton_clicked()
{
    ui->mainStack->setCurrentIndex(0);
}

void KeyboardPane::on_availableKeyboardLayouts_itemActivated(QListWidgetItem *item)
{
    QStringList currentLayouts = d->settings.value("input/layout", "us(basic)").toString().split(",");
    currentLayouts.append(item->data(Qt::UserRole).toString());

    QListWidgetItem* newItem = new QListWidgetItem();
    newItem->setText(item->text());
    newItem->setData(Qt::UserRole, item->data(Qt::UserRole));
    ui->selectedLayouts->insertItem(ui->selectedLayouts->count() - 1, newItem);
    d->settings.setValue("input/layout", currentLayouts.join(","));

    ui->mainStack->setCurrentIndex(0);
    emit loadNewKeyboardLayoutMenu();
}

void KeyboardPane::on_selectedLayouts_customContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem* item = ui->selectedLayouts->itemAt(pos);
    if (item->data(Qt::UserRole).toString() != "add" && ui->selectedLayouts->count() > 2) {
        QMenu* menu = new QMenu();
        menu->addSection(tr("For %1").arg(item->text()));
        menu->addAction(QIcon::fromTheme("edit-delete"), tr("Remove"), [=] {
            QStringList currentLayouts = d->settings.value("input/layout", "us(basic)").toString().split(",");

            int i = ui->selectedLayouts->row(item);
            currentLayouts.removeAt(i);
            ui->selectedLayouts->takeItem(i);
            delete item;

            d->settings.setValue("input/layout", currentLayouts.join(","));
            loadNewKeyboardLayoutMenu();
        });
        menu->exec(ui->selectedLayouts->mapToGlobal(pos));
    }
}

void KeyboardPane::on_SuperKeyOpenGatewaySwitch_toggled(bool checked)
{
    d->settings.setValue("input/superkeyGateway", checked);
}
