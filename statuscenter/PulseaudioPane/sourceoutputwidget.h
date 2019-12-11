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
#ifndef SOURCEOUTPUTWIDGET_H
#define SOURCEOUTPUTWIDGET_H

#include <QWidget>

namespace PulseAudioQt {
    class SourceOutput;
}

namespace Ui {
    class SourceOutputWidget;
}

struct SourceOutputWidgetPrivate;
class SourceOutputWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SourceOutputWidget(PulseAudioQt::SourceOutput* sourceOutput, QWidget *parent = nullptr);
        ~SourceOutputWidget();

        enum ListeningState {
            NotListening = 0,
            BlockedFromListening = 1,
            Listening = 2
        };

        PulseAudioQt::SourceOutput* sourceOutput();

        ListeningState listeningState();

    signals:
        void listeningStateChanged();

    private slots:
        void updateName();

        void on_sourceSelectionButton_clicked();

        void on_muteButton_toggled(bool checked);

    private:
        Ui::SourceOutputWidget *ui;
        SourceOutputWidgetPrivate* d;
};

#endif // SOURCEOUTPUTWIDGET_H
