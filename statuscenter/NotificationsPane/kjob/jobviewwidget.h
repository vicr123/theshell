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
#ifndef JOBVIEWWIDGET_H
#define JOBVIEWWIDGET_H

#include <QFrame>

namespace Ui {
    class JobViewWidget;
}

class JobViewWidget : public QFrame
{
        Q_OBJECT

    public:
        explicit JobViewWidget(QString title, QString icon, int capabilities, QWidget *parent = nullptr);
        ~JobViewWidget();

        void setSuspended(bool suspended);
        void setInfoMessage(QString message);
        void setDescriptionField(uint number, QString name, QString value);
        void clearDescriptionField(uint number);
        void setPercent(uint percent);

    signals:
        void terminate();
        void suspend();

    private slots:
        void on_pauseButton_clicked();

        void on_cancelButton_clicked();

    private:
        Ui::JobViewWidget *ui;
};

#endif // JOBVIEWWIDGET_H
