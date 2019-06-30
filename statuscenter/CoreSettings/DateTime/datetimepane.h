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
#ifndef DATETIMEPANE_H
#define DATETIMEPANE_H

#include <tstackedwidget.h>
#include <statuscenterpaneobject.h>
#include <tpromise.h>

namespace Ui {
    class DateTimePane;
}

struct DateTimePanePrivate;
class DateTimePane : public tStackedWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit DateTimePane(QWidget *parent = nullptr);
        ~DateTimePane();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void on_twentyFourHourTimeButton_toggled(bool checked);

        void on_twelveHourTimeButton_toggled(bool checked);

        void on_changeTimeButton_clicked();

        void on_backButton_clicked();

        void on_backButton_2_clicked();

        void on_changeTimezoneButton_clicked();

        void on_timezonesList_activated(const QModelIndex &index);

        void on_setDateTimeButton_clicked();

        void on_ntpEnabledSwitch_toggled(bool checked);

        void on_searchTimezones_textChanged(const QString &arg1);

    private:
        Ui::DateTimePane *ui;

        DateTimePanePrivate* d;
        tPromise<void>* launchDateTimeService();
        tPromise<void>* checkPolkit(QString action);
};

#endif // DATETIMEPANE_H
