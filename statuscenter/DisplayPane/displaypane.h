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
#ifndef DISPLAYPANE_H
#define DISPLAYPANE_H

#include <QWidget>
#include <statuscenterpaneobject.h>

namespace Ui {
    class DisplayPane;
}

struct DisplayPanePrivate;
class DisplayPane : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit DisplayPane(QWidget *parent = nullptr);
        ~DisplayPane();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    public slots:
        void on_dpi100_toggled(bool checked);

        void on_dpi150_toggled(bool checked);

        void on_dpi200_toggled(bool checked);

        void on_dpi300_toggled(bool checked);

        void on_sunlightRedshift_toggled(bool checked);

        void on_startRedshift_timeChanged(const QTime &time);

        void on_endRedshift_timeChanged(const QTime &time);

        void on_redshiftIntensity_sliderMoved(int position);

        void on_redshiftIntensity_sliderReleased();

        void on_redshiftIntensity_valueChanged(int value);

        void on_redshiftPause_toggled(bool checked);

        void updateRedshiftTime();

        void updateRedshiftTime(double latitude, double longitude);

    private:
        Ui::DisplayPane *ui;

        DisplayPanePrivate* d;
};

#endif // DISPLAYPANE_H
