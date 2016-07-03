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

    geolocationSource = QGeoPositionInfoSource::createDefaultSource(this);
    connect(geolocationSource, &QGeoPositionInfoSource::positionUpdated, [=](QGeoPositionInfo position) {
        this->currentCoordinates = position.coordinate();
    });
    geolocationSource->startUpdates();

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

    /*speechProc = new QProcess(this);
    speechProc->setProcessChannelMode(QProcess::MergedChannels);
    connect(speechProc, SIGNAL(readyRead()), this, SLOT(outputAvailable()));
    speechProc->start("pocketsphinx_continuous -inmic yes");
    speechProc->waitForStarted();
    speechProc->waitForFinished(-1);*/

    recorder = new QAudioRecorder(this);
    QAudioEncoderSettings settings;
    settings.setCodec("audio/PCM");
    settings.setChannelCount(1);
    settings.setSampleRate(16000);
    settings.setQuality(QMultimedia::NormalQuality);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);

    QAudioProbe* probe = new QAudioProbe(this);
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(soundBuffer(QAudioBuffer)));
    connect(recorder, SIGNAL(destroyed(QObject*)), probe, SLOT(deleteLater()));
    probe->setSource(recorder);
    oldLoudness = 0;

    recorder->setEncodingSettings(settings, QVideoEncoderSettings(), "wav");
    recorder->setOutputLocation(QUrl::fromLocalFile(QDir::homePath() + "/.thewaveSpeech.wav"));
    recorder->record();

    endListenTimer.start();
    startListeningSound->play();
    emit outputSpeech("Go for it!");
    emit startedListening();
}

void theWaveWorker::soundBuffer(QAudioBuffer buffer) {
    //Get loudness of sound
    qreal loudness = 0;
    qreal peak_value(USHRT_MAX);

    for (int i = 0; i < buffer.frameCount(); i++) {
        qreal value = qAbs(qreal(buffer.constData<qint16>()[i]));
        if (value > loudness) {
            loudness = value;
        }
    }

    loudness /= peak_value;

    emit loudnessChanged(loudness);

    //If loudness has changed considerably, reset timer
    if (oldLoudness - 0.01 > loudness || oldLoudness + 0.01 < loudness) {
        oldLoudness = loudness;
        endListenTimer.restart();
        //qDebug() << "Loudness Reset!";
    }

    //If loudness has stayed the same for a second, stop listening.
    if (endListenTimer.elapsed() >= 1000) {
        //qDebug() << "Stop listening now!";
        endAndProcess();
    }
}

void theWaveWorker::endAndProcess() {
    recorder->stop();
    //delete recorder;
    recorder = NULL;

    okListeningSound->play();
    emit outputSpeech("Processing...");
    emit stoppedListening();

    emit loudnessChanged(-1);

    speechProc = new QProcess(this);
    speechProc->setProcessChannelMode(QProcess::MergedChannels);
    connect(speechProc, SIGNAL(readyRead()), this, SLOT(outputAvailable()));
    speechProc->start("pocketsphinx_continuous -infile \"" + QDir::homePath() + "/.thewaveSpeech.wav\"");
    speechProc->waitForStarted();
}

void theWaveWorker::outputAvailable() {
    buffer.append(QString(speechProc->readAll()));

    if (buffer.endsWith("\n")) {
        QStringList bufferOutput = buffer.split("\n");
        for (QString outputString : bufferOutput) {
            qDebug() << outputString;
            if (outputString.startsWith("INFO:")) {
            } else if (outputString.startsWith("-") || outputString == "Current configuration:" || outputString.startsWith("[NAME]")) {
            } else {
                if (outputString != "") {
                    emit outputSpeech(outputString);
                    processSpeech(outputString);
                    disconnect(speechProc, SIGNAL(readyRead()), this, SLOT(outputAvailable()));
                    speechProc->kill();
                    delete speechProc;
                    emit loudnessChanged(-2);
                }
            }
        }
        buffer = "";
    }
}

void theWaveWorker::processSpeech(QString speech, bool voiceFeedback) {
    QString name = settings.value("thewave/name").toString();
    if (resetOnNextBegin) {
        resetOnNextBegin = false;
        emit resetFrames();
    }
    QString parse = speech.toLower();
    if (speech == "") {
        emit outputResponse("That flew past me. Try again.");
        speak("That flew past me. Try again.", voiceFeedback);
    } else {
        switch (this->state) {
        case Idle:
            if (parse.contains("hello")) {
                if (name == "") {
                    emit outputResponse("Hey there! How are you today?");
                    speak("Hey there! How are you today?");
                } else {
                    emit outputResponse("Hello " + name + "! How are you today?");
                    speak("Hello " + name + "! How are you today?");
                }
            } else if (parse == "who am i") {
                if (name == "") {
                    emit outputResponse("I'm not quite sure. Tell me in my settings.");
                    speak("I'm not quite sure. Tell me in my settings.");
                } else {
                    emit outputResponse("You're " + name + ", aren't you?");
                    speak("You're " + name + ", aren't you?");
                }
            } else if (parse == "who are you") {
                emit outputResponse("Me? I'm theWave. Pleased to meet you.");
                speak("Me? I'm theWave. Pleased to meet you.");
            } else if (parse == "fuck you") {
                emit outputResponse("Did I do something wrong?");
                speak("Did I do something wrong?");
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
            } else if (parse.contains("flight")) {
                emit showFlightFrame("Unknown Airline 000");
                emit outputResponse("I can't find flight information at the moment.");
                speak("I can't find flight information at the moment.");
                resetOnNextBegin = true;
            } else if (parse == "where am i") {
                geolocationSource->requestUpdate();
                QGeoCoordinate coordinates = this->currentCoordinates;
                QString output;
                if (coordinates.isValid()) {
                    output = "You're at " + QString::number(coordinates.latitude()) + " " + QString::number(coordinates.longitude());
                } else {
                    output = "I'm not quite sure where you are.";
                }
                emit outputResponse(output);
                speak(output);
            /*} else if (parse.contains("weather")) {
                //Get the location the user wants
                QString location = ""; //If Location is blank, get current location.

                if (location == "") { //Use current location
                    geolocationSource->requestUpdate();
                    if (currentCoordinates.isValid()) {

                    } else {
                        emit outputResponse("I'm not sure where you are, so I can't get weather information. Try saying a city name. Sorry about that.");
                        speak("I'm not sure where you are, so I can't get weather information. Try saying a city name. Sorry about that.");
                    }
                }*/
            } else if (parse.contains("happy birthday")) {
                QString output = "It's not my birthday. But I'm assuming it's yours.";
                if (name != "") {
                    output = "It's not my birthday. But I'm assuming it's yours. Happy Birthday " + name + "!";
                }
                emit outputResponse(output);
                speak(output);
            } else {
                emit outputResponse("Looking online for information...");
                speak("Looking online for information...");

                bool isInfoFound = false;

                if (settings.value("thewave/wikipediaSearch", true).toBool()) {
                    QEventLoop eventLoop;

                    QNetworkRequest request;
                    QUrl requestUrl("https://en.wikipedia.org/w/api.php?action=query&titles=" + speech.replace(" ", "%20") + "&format=xml&prop=extracts&redirects=true&exintro=true");
                    request.setUrl(requestUrl);
                    request.setHeader(QNetworkRequest::UserAgentHeader, "theWave/2.1 (vicr12345@gmail.com)");
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
                    speak("I'm sorry, that's not a valid time. I need a time given in hours, minutes and or seconds. Otherwise, you can say \"Stop.\"", voiceFeedback);
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
    while (speechPlaying) {
        QApplication::processEvents();
    }

    speechPlaying = true;
    if (settings.value("thewave/ttsEngine").toString() == "pico2wave" && QFile("/usr/bin/pico2wave").exists()) {
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
            speechPlaying = false;
        });
        //QSound sound();
        //sound.play();

        //while (!sound.isFinished()) {
            //QApplication::processEvents();
        //}
    } else if (settings.value("thewave/ttsEngine").toString() == "espeak" && QFile("/usr/bin/espeak").exists()) {
        QProcess *s = new QProcess(this);
        s->start("espeak \"" + speech + "\"");

        if (restartOnceComplete && !stopEverything) {
            connect(s, SIGNAL(finished(int)), this, SLOT(begin()));
        }
        speechPlaying = false;
    } else {
        QProcess *s = new QProcess(this);
        s->start("festival --tts");
        s->write(speech.toUtf8());
        s->closeWriteChannel();

        if (restartOnceComplete && !stopEverything) {
            connect(s, SIGNAL(finished(int)), this, SLOT(begin()));
        }
        speechPlaying = false;
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
