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
#ifndef SHORTCUTEDIT_H
#define SHORTCUTEDIT_H

#include <QKeySequenceEdit>
#include <QSettings>

struct ShortcutEditPrivate;
class ShortcutEdit : public QWidget
{
        Q_OBJECT
    public:
        explicit ShortcutEdit(QSettings* settings, QString setting, QString keyName, int index, QKeySequence defaultShortcut, QWidget *parent = nullptr);
        ~ShortcutEdit();

    signals:
        void activated();

    public slots:

    private:
        ShortcutEditPrivate* d;

        void focusInEvent(QFocusEvent *event);
        void focusOutEvent(QFocusEvent *event);
        void paintEvent(QPaintEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void keyReleaseEvent(QKeyEvent *event);
        QSize sizeHint() const;

        QPixmap getKeyIcon(QString key);
        bool isModifierKey(Qt::Key key);
        void editingDone();
};

#endif // SHORTCUTEDIT_H
