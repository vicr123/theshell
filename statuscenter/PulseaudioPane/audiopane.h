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
#ifndef AUDIOPANE_H
#define AUDIOPANE_H

#include <QWidget>
#include <pulse/subscribe.h>
#include <pulse/introspect.h>

#include <statuscenterpaneobject.h>

namespace Ui {
    class AudioPane;
}

struct AudioPanePrivate;
class AudioPane : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit AudioPane(QWidget *parent = nullptr);
        ~AudioPane();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void on_backButton_clicked();

        void connectToPulse();

        void on_volumeOverdrive_toggled(bool checked);

        void on_soundThemeComboBox_currentIndexChanged(int index);

        void addSoundSetting(QString name, QString soundName, QString soundPermission);

        void on_turnOffQuietModeOutputDevicesButton_clicked();

        void listeningStateChanged();

    signals:
        void defaultSinkChanged(QString defaultSink);

    private:
        Ui::AudioPane *ui;

        AudioPanePrivate* d;

        void changeEvent(QEvent* event);
};

#endif // AUDIOPANE_H
