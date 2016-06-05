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

class theWaveWorker : public QObject
{
    Q_OBJECT

    enum SpeechState {
        Idle,
        TimerGetTime,
    };

    QMap<QString, int> numberDictionary;

public:
    explicit theWaveWorker(QObject *parent = 0);
    ~theWaveWorker();

signals:
    void outputSpeech(QString);
    void outputResponse(QString);
    void outputFrame(QFrame *);
    void complete();
    void startedListening();
    void stoppedListening();
    void finished();

    void resetFrames();
    void showCallFrame(bool emergency);
    void showMessageFrame();
    void showHelpFrame();
    void showWikipediaFrame(QString title, QString text);
    void launchApp(QString app);

    void setTimer(QTime);

public slots:
    void begin();

    void processSpeech(QString speech, bool voiceFeedback = true);

    void quit();

    void soundBuffer(QAudioBuffer buffer);

private slots:
    void outputAvailable();

private:
    QProcess *speechProc;
    QString buffer;

    void speak(QString speech, bool restartOnceComplete = false);
    SpeechState state = Idle;

    QSoundEffect* startListeningSound, *okListeningSound, *errorListeningSound, *stopListeningSound;
    QAudioRecorder* recorder = NULL;

    QGeoPositionInfoSource* geolocationSource;
    QGeoCoordinate currentCoordinates;

    bool stopEverything = false;
    bool resetOnNextBegin = false;

    QSettings settings;
};

#endif // THEWAVEWORKER_H
