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

signals:
    void outputSpeech(QString);
    void outputResponse(QString);
    void outputFrame(QFrame *);
    void complete();
    void startedListening();
    void stoppedListening();
    void finished();

    void resetFrames();
    void showCallFrame();
    void showMessageFrame();

    void setTimer(QTime);

public slots:
    void begin();

    void processSpeech(QString speech);

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

    bool stopEverything = false;
    bool resetOnNextBegin = false;
};

#endif // THEWAVEWORKER_H
