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

theWaveWorker::~theWaveWorker() {
    emit finished();
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

    /*recorder = new QAudioRecorder(this);
    QAudioEncoderSettings settings;
    settings.setCodec("audio/wav");
    settings.setQuality(QMultimedia::HighQuality);

    QAudioProbe* probe = new QAudioProbe(this);
    probe->setSource(recorder);
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(soundBuffer(QAudioBuffer)));

    recorder->setEncodingSettings(settings);
    recorder->setOutputLocation(QUrl::fromLocalFile("/home/victor/sound.wav"));
    recorder->record();

    emit outputSpeech("Go for it!");
    emit startedListening();*/
}

void theWaveWorker::soundBuffer(QAudioBuffer buffer) {
    float avg;
    const float* data = buffer.data<float>();
    for (int i = 0; i < buffer.byteCount(); i++) {
        avg = avg + data[i];
    }

    avg = avg / buffer.byteCount();
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

void theWaveWorker::processSpeech(QString speech, bool voiceFeedback) {
    if (resetOnNextBegin) {
        resetOnNextBegin = false;
        emit resetFrames();
    }
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
                emit showCallFrame(false);
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
                    speak("How long do you want this timer to be set for?", voiceFeedback);
                    state = TimerGetTime;
                } else {
                    goto TimerGetTime;
                }
            } else if (parse.contains("help") || parse.contains("what can you do")) {
                emit outputResponse("I can do some things. Try asking me something from this list.");
                speak("I can do some things. Try asking me something from this list.");
                emit showHelpFrame();
                resetOnNextBegin = true;
            } else if (parse.startsWith("start", Qt::CaseInsensitive) || parse.startsWith("launch", Qt::CaseInsensitive)) {
                emit launchApp(parse.remove(0, 6));
                resetOnNextBegin = true;
            } else {
                emit outputResponse("Looking online for information...");
                speak("Looking online for information...");

                bool isInfoFound = false;

                if (settings.value("thewave/wikipediaSearch", true).toBool()) {
                    QEventLoop eventLoop;

                    QNetworkRequest request;
                    QUrl requestUrl("https://en.wikipedia.org/w/api.php?action=query&titles=" + parse.replace(" ", "%20") + "&format=xml&prop=extracts&redirects=true&exintro=true");
                    request.setUrl(requestUrl);
                    request.setHeader(QNetworkRequest::UserAgentHeader, "theWave/2.0 (vicr12345@gmail.com)");
                    QNetworkAccessManager networkManager;
                    connect(&networkManager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
                    QNetworkReply* NetworkReply = networkManager.get(request);

                    eventLoop.exec();

                    QString reply(NetworkReply->readAll());
                    qDebug() << reply;

                    if (reply.contains("title=") && !reply.contains("missing=\"\"")) {
                        isInfoFound = true;
                        QString title = reply.split("title=\"").at(1).split("\"").at(0);
                        QString text;

                        text = "<!DOCTYPE HTML><html><head></head><body>" + reply.split("<extract xml:space=\"preserve\">").at(1).split("</extract>").at(0) + "</body></html>";
                        text.replace("&lt;", "<");
                        text.replace("&gt;", ">");
                        text.replace("&quot;", "\"");
                        text.replace("&amp;", "&");

                        emit showWikipediaFrame(title, text);
                        emit outputResponse("I found some information. Take a look.");
                        speak("I found some information. Take a look.");
                        resetOnNextBegin = true;
                    }

                }

                if (!isInfoFound) {
                    emit outputResponse("Unfortunately, I don't understand you. Try again.");
                    speak("Unfortunately, I don't understand you. Try again.");
                    errorListeningSound->play();
                }
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
    if (settings.value("thewave/ttsEngine").toString() == "pico2wave" && QFile("/usr/bin/pico2wave").exists()) {
        /*QProcess *s = new QProcess(this);
        s->setInputChannelMode(QProcess::);
        s->start();
        s->waitForStarted();
        s->waitForFinished();*/
        QString command = "pico2wave -w=\"" + QDir::homePath() + "/.thewavevoice.wav\" \"" + speech + "\"";
        QProcess::execute(command);

        QSoundEffect* sound = new QSoundEffect();
        sound->setSource(QUrl::fromLocalFile(QDir::homePath() + "/.thewavevoice.wav"));
        sound->play();
        connect(sound, &QSoundEffect::playingChanged, [=]() {
            if (!sound->isPlaying()) {
                sound->deleteLater();
            }
        });
        connect(sound, &QSoundEffect::destroyed, [=]() {
            bool success = QFile(QDir::homePath() + "/.thewavevoice.wav").remove();

            if (restartOnceComplete && !stopEverything) {
                begin();
            }
        });
        //QSound sound();
        //sound.play();

        //while (!sound.isFinished()) {
            //QApplication::processEvents();
        //}
    } else {
        QProcess *s = new QProcess(this);
        s->start("festival --tts");
        s->write(speech.toUtf8());
        s->closeWriteChannel();

        if (restartOnceComplete && !stopEverything) {
            connect(s, SIGNAL(finished(int)), this, SLOT(begin()));
        }
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
