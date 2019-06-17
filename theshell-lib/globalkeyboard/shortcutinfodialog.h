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
#ifndef SHORTCUTINFODIALOG_H
#define SHORTCUTINFODIALOG_H

#include <QDialog>

namespace Ui {
    class ShortcutInfoDialog;
}

class GlobalKeyboardKey;
struct ShortcutInfoDialogPrivate;
class ShortcutInfoDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit ShortcutInfoDialog(QWidget *parent = nullptr);
        ~ShortcutInfoDialog();

        void showChords(QKeySequence currentKey, QList<GlobalKeyboardKey*> availableKeys, QString status);

    public slots:
        void show();

    private:
        Ui::ShortcutInfoDialog *ui;
        ShortcutInfoDialogPrivate* d;

        void paintEvent(QPaintEvent* event);
};

#endif // SHORTCUTINFODIALOG_H
