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
#ifndef KEYBOARDPANE_H
#define KEYBOARDPANE_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
    class KeyboardPane;
}

struct KeyboardPanePrivate;
class KeyboardPane : public QWidget
{
        Q_OBJECT

    public:
        explicit KeyboardPane(QWidget *parent = nullptr);
        ~KeyboardPane();

    signals:
        void loadNewKeyboardLayoutMenu();

    private slots:
        void KeyboardLayoutsMoved();

        void on_selectedLayouts_itemActivated(QListWidgetItem *item);

        void on_backButton_clicked();

        void on_availableKeyboardLayouts_itemActivated(QListWidgetItem *item);

        void on_selectedLayouts_customContextMenuRequested(const QPoint &pos);

        void on_SuperKeyOpenGatewaySwitch_toggled(bool checked);

    private:
        Ui::KeyboardPane *ui;

        KeyboardPanePrivate* d;
        void changeEvent(QEvent* event);
};

#endif // KEYBOARDPANE_H
