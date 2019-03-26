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
#ifndef MOUSEPANE_H
#define MOUSEPANE_H

#include <QWidget>

namespace Ui {
    class MousePane;
}

struct MousePanePrivate;
class MousePane : public QWidget
{
        Q_OBJECT

    public:
        explicit MousePane(QWidget *parent = nullptr);
        ~MousePane();

    private slots:
        void applySettings();

        void on_leftPrimaryButton_toggled(bool checked);

        void on_rightPrimaryButton_toggled(bool checked);

        void on_speedSlider_valueChanged(int value);

        void on_speedSlider_sliderReleased();

        void on_tapToClick_toggled(bool checked);

        void on_naturalMouseScrolling_toggled(bool checked);

        void on_naturalTouchpadScrolling_toggled(bool checked);

    private:
        Ui::MousePane *ui;

        MousePanePrivate* d;
        bool eventFilter(QObject* watched, QEvent* event);
};

#endif // MOUSEPANE_H
