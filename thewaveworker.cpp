#include "thewaveworker.h"

theWaveWorker::theWaveWorker(QObject *parent) : QObject(parent)
{

    startListeningSound = new QSoundEffect();
    startListeningSound->setSource(QUrl("qrc:/sounds/listening.wav"));
    okListeningSound = new QSoundEffect();
    okListeningSound->setSource(QUrl("qrc:/sounds/ok.wav"));
    errorListeningSound = new QSoundEffect();
    errorListeningSound->setSource(QUrl("qrc:/sounds/what.wav"));
    stopListeningSound = new QSoundEffect();
    stopListeningSound->setSource(QUrl("qrc:/sounds/notlistening.wav"));

    numberDictionary["one"] = 1;
    numberDictionary["two"] = 2;
    numberDictionary["three"] = 3;
    numberDictionary["four"] = 4;
    numberDictionary["five"] = 5;
    numberDictionary["six"] = 6;
    numberDictionary["seven"] = 7;
    numberDictionary["eight"] = 8;
    numberDictionary["nine"] = 9;
    numberDictionary["ten"] = 10;
    numberDictionary["eleven"] = 11;
    numberDictionary["twelve"] = 12;
    numberDictionary["thirteen"] = 13;
    numberDictionary["fourteen"] = 14;
    numberDictionary["fifteen"] = 15;
    numberDictionary["sixteen"] = 16;
    numberDictionary["seventeen"] = 17;
    numberDictionary["eighteen"] = 18;
    numberDictionary["nineteen"] = 19;
    numberDictionary["twenty"] = 20;
    numberDictionary["thirty"] = 30;
    numberDictionary["forty"] = 40;
    numberDictionary["fifty"] = 50;
    numberDictionary["sixty"] = 60;
    numberDictionary["seventy"] = 70;
    numberDictionary["eighty"] = 80;
    numberDictionary["ninety"] = 90;
    numberDictionary["hundred"] = 100;
    numberDictionary["thousand"] = 1000;
    numberDictionary["million"] = 100000;
}

void theWaveWorker::begin() {
    if (resetOnNextBegin) {
        resetOnNextBegin = false;
        emit resetFrames();
    }
    speechProc = new QProcess(this);
    speechProc->setProcessChannelMode(QProcess::MergedChannels);
    connect(speechProc, SIGNAL(readyRead()), this, SLOT(outputAvailable()));
    speechProc->start("pocketsphinx_continuous -inmic yes");
    speechProc->waitForStarted();
    speechProc->waitForFinished(-1);
}

void theWaveWorker::outputAvailable() {
    buffer.append(QString(speechProc->readAll()));

    if (buffer.endsWith("\n")) {
        QStringList bufferOutput = buffer.split("\n");
        for (QString outputString : bufferOutput) {
            qDebug() << outputString;
            if (outputString.startsWith("INFO:")) {
                if (outputString.contains("Ready")) {
                    //QSound::play(":/sounds/listening.wav");
                    startListeningSound->play();
                    emit outputSpeech("Go for it!");
                    emit startedListening();
                } else if (outputString.contains("Listening")) {
                    emit outputSpeech("Listening...");
                } else if (outputString.contains("words recognized") && outputString.contains("ngram_search_fwdtree")) {
                    emit outputSpeech("Processing...");
                    okListeningSound->play();
                }
            } else if (outputString.startsWith("-") || outputString == "Current configuration:" || outputString.startsWith("[NAME]")) {
            } else {
                if (outputString != "") {
                    emit outputSpeech(outputString);
                    emit stoppedListening();
                    speechProc->kill();

                    processSpeech(outputString);
                }
            }
        }
        buffer = "";
    }
}

void theWaveWorker::processSpeech(QString speech) {
    QString parse = speech.toLower();
    if (speech == "") {
        emit outputResponse("That flew past me. Try again.");
        speak("That flew past me. Try again.", true);
    } else {
        switch (this->state) {
        case Idle:
            if (parse.contains("hello")) {
                emit outputResponse("Hey there! How are you today?");
                speak("Hey there! How are you today?");
            } else if (parse == "who are you") {
                emit outputResponse("Me? I'm theWave. Pleased to meet you.");
                speak("Me? I'm theWave. Pleased to meet you.");
            } else if (parse.contains("call")) {
                emit outputResponse("Unfortunately, I can't place a call from this device.");
                speak("Unfortunately, I can't place a call from this device.");
                emit showCallFrame();
                resetOnNextBegin = true;
            } else if (parse.contains("text") || parse.contains("message")) {
                emit outputResponse("Unfortunately, I can't send text messages from this device. This functionality may come later for IM applications.");
                speak("Unfortunately, I can't send text messages from this device.");
                emit showMessageFrame();
                resetOnNextBegin = true;
            } else if (parse.contains("timer") || parse.contains("countdown") || parse.contains("count down")) {
                QStringList parseSpace = parse.split(" ");
                int currentNumber = 0, hour = 0, minute = 0, second = 0;
                for (QString part : parseSpace) {
                    if (numberDictionary.contains(part)) {
                        currentNumber = currentNumber + numberDictionary.value(part);
                    } else if (part.contains("hour")) {
                        hour = hour + currentNumber;
                        currentNumber = 0;
                    } else if (part.contains("minute")) {
                        minute = minute + currentNumber;
                        currentNumber = 0;
                    } else if (part.contains("second")) {
                        second = second + currentNumber;
                        currentNumber = 0;
                    }
                }

                if (hour == 0 && minute == 0 && second == 0) {
                    emit outputResponse("How long do you want this timer to be set for?");
                    speak("How long do you want this timer to be set for?", true);
                    state = TimerGetTime;
                } else {
                    goto TimerGetTime;
                }
            } else {
                emit outputResponse("Unfortunately, I don't understand you. Try again.");
                speak("Unfortunately, I don't understand you. Try again.");
                errorListeningSound->play();
            }
            break;
        case TimerGetTime:
            TimerGetTime:
            if (parse.contains("hour") || parse.contains("minute") || parse.contains("second")) {
                QStringList parseSpace = parse.split(" ");
                int currentNumber = 0, hour = 0, minute = 0, second = 0;
                for (QString part : parseSpace) {
                    if (numberDictionary.contains(part)) {
                        currentNumber = currentNumber + numberDictionary.value(part);
                    } else if (part.contains("hour")) {
                        hour = hour + currentNumber;
                        currentNumber = 0;
                    } else if (part.contains("minute")) {
                        minute = minute + currentNumber;
                        currentNumber = 0;
                    } else if (part.contains("second")) {
                        second = second + currentNumber;
                        currentNumber = 0;
                    }
                }
                if (hour >= 60 || minute >= 60 || second >= 60) {
                    emit outputResponse("I'm sorry, that's not a valid time. I need a time given in hours, minutes and/or seconds. Otherwise, you can say \"Stop.\"");
                    speak("I'm sorry, that's not a valid time. I need a time given in hours, minutes and or seconds. Otherwise, you can say \"Stop.\"", true);
                    state = TimerGetTime;
                } else {
                    QString compiledSpeech = "I've set the timer for ";
                    bool includeAnd = false;
                    if (hour != 0) {
                        compiledSpeech.append(QString::number(hour) + " hours, ");
                        includeAnd = true;
                    }
                    if (minute != 0) {
                        compiledSpeech.append(QString::number(minute) + " minutes, ");
                        includeAnd = true;
                    }
                    if (second != 0) {
                        compiledSpeech.append(QString::number(second) + " seconds.");
                    }

                    QTime t(hour, minute, second);
                    emit setTimer(t);
                    emit outputResponse(compiledSpeech + " You can check your timer next to the clock.");
                    speak(compiledSpeech);
                    state = Idle;
                }
            } else if (parse.contains("cancel") || parse.contains("stop")) {
                state = Idle;
                emit outputResponse("OK, I cancelled the timer.");
                speak("OK, I cancelled the timer.");
            } else {
                emit outputResponse("I'm sorry, I don't understand what you mean. I need a time given in hours, minutes and/or seconds. Otherwise, you can say \"Stop.\"");
                speak("I'm sorry, I don't understand what you mean. I need a time given in hours, minutes and or seconds. Otherwise, you can say \"Stop.\"", true);
                state = TimerGetTime;
            }
        }
    }
}

void theWaveWorker::speak(QString speech, bool restartOnceComplete) {
    QProcess *s = new QProcess(this);
    s->start("festival --tts");
    s->write(speech.toUtf8());
    s->closeWriteChannel();

    if (restartOnceComplete && !stopEverything) {
        connect(s, SIGNAL(finished(int)), this, SLOT(begin()));
    }
}

void theWaveWorker::quit() {
    speechProc->kill();
    stopEverything = true;
    stopListeningSound->play();
    connect(stopListeningSound, &QSoundEffect::playingChanged, [=]() {
        if (!stopListeningSound->isPlaying()) {
            delete startListeningSound;
            delete okListeningSound;
            delete errorListeningSound;
            delete stopListeningSound;
        }
    });
    //delete this;
}
