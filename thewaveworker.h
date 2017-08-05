/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef THEWAVEWORKER_H
#define THEWAVEWORKER_H

#include <QObject>
#include <QProcess>
#include <QFrame>
#include <QSoundEffect>
#include <QSound>
#include <QUrl>
#include <QDebug>
#include <QMap>
#include <QTime>
#include <QAudioRecorder>
#include <QAudioEncoderSettings>
#include <QAudioProbe>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QXmlSimpleReader>
#include <QXmlInputSource>
#include <QEventLoop>
#include <QDir>
#include <QApplication>
#include <QGeoPositionInfoSource>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <qmath.h>
#include <KF5/AkonadiCore/control.h>
#include <KF5/AkonadiCore/servermanager.h>
#include <KF5/AkonadiCore/session.h>
#include "menu.h"

class Menu;

class theWaveWorker : public QObject
{
    Q_OBJECT

    enum SpeechState {
        Idle,
        TimerGetTime,
        emailGetRecipient,
        emailGetSubject,
        emailGetBody,
        emailConfirm
    };

    QMap<QString, float> numberDictionary;

public:
    explicit theWaveWorker(QObject *parent = 0);
    ~theWaveWorker();

    bool isDisabled();

signals:
    void outputSpeech(QString);
    void outputResponse(QString);
    void outputHint(QString);
    void loudnessChanged(qreal loudness);
    void outputFrame(QFrame *);
    void complete();
    void startedListening();
    void stoppedListening();
    void showBigListenFrame();
    void hideBigListenFrame();
    void finished();

    void resetFrames();
    void showCallFrame(bool emergency);
    void showMessageFrame();
    void showHelpFrame();
    void showWikipediaFrame(QString title, QString text);
    void launchApp(QString app);
    void doLaunchApp(QString app);
    void showFlightFrame(QString flight);
    void showSettingsFrame(QIcon icon, QString setting, bool isOn);
    void showMathematicsFrame(QString expression, QString answer);
    void showMediaFrame(QPixmap art, QString title, QString artist, bool isPlaying);

    void setTimer(QTime);

public slots:
    void begin();

    void endAndProcess();

    void processSpeech(QString speech, bool voiceFeedback = true);

    void quit();

    void soundBuffer(QAudioBuffer buffer);

    bool launchAkonadi();

    void launchAppReply(QString app);

    void launchApp_disambiguation(QStringList apps);

    void currentSettingChanged(bool isOn);

private slots:
    void outputAvailable();

    void SetSetting(QString setting, bool isOn);

private:
    QProcess *speechProc;
    QString buffer;
    QTime endListenTimer;
    qreal oldLoudness = 0;
    qreal maxLoudnessForSession = 0;
    bool isListeningAfterLoudnessChange = false;

    void speak(QString speech, bool restartOnceComplete = false);
    SpeechState state = Idle;

    QSoundEffect* startListeningSound, *okListeningSound, *errorListeningSound, *stopListeningSound;
    QAudioRecorder* testRecorder = NULL;
    QAudioRecorder* recorder = NULL;
    QAudioProbe* probe = NULL;

    QGeoPositionInfoSource* geolocationSource;
    QGeoCoordinate currentCoordinates;

    bool stopEverything = false;
    bool resetOnNextBegin = false;
    bool speechPlaying = false;
    bool disabled = false;
    bool isRunning = false;
    bool noVoiceInput = true;

    QString currentSetting;

    QSettings settings;
};

#endif // THEWAVEWORKER_H
