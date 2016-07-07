#include "thewaveworker.h"

extern MainWindow* MainWin;

theWaveWorker::theWaveWorker(QObject *parent) : QObject(parent)
{
    startListeningSound = new QSoundEffect(this);
    startListeningSound->setSource(QUrl("qrc:/sounds/listening.wav"));
    okListeningSound = new QSoundEffect(this);
    okListeningSound->setSource(QUrl("qrc:/sounds/ok.wav"));
    errorListeningSound = new QSoundEffect(this);
    errorListeningSound->setSource(QUrl("qrc:/sounds/what.wav"));
    stopListeningSound = new QSoundEffect(this);
    stopListeningSound->setSource(QUrl("qrc:/sounds/notlistening.wav"));

    geolocationSource = QGeoPositionInfoSource::createDefaultSource(this);
    connect(geolocationSource, &QGeoPositionInfoSource::positionUpdated, [=](QGeoPositionInfo position) {
        this->currentCoordinates = position.coordinate();
    });
    geolocationSource->startUpdates();

    numberDictionary["quarter"] = 0.25;
    numberDictionary["half"] = 0.5;
    numberDictionary["zero"] = 0;
    numberDictionary["a"] = 1;
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
    numberDictionary["million"] = 1000000;

    numberDictionary["pi"] = M_PI;
    numberDictionary["tau"] = M_PI * 2;
    numberDictionary["e"] = M_E;

    recorder = new QAudioRecorder(this);
    QAudioEncoderSettings settings;
    settings.setCodec("audio/PCM");
    settings.setChannelCount(1);
    settings.setSampleRate(16000);
    settings.setQuality(QMultimedia::NormalQuality);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);

    probe = new QAudioProbe(this);
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(soundBuffer(QAudioBuffer)));
    oldLoudness = 0;

    QFile(QDir::homePath() + "/.thewaveSpeech.wav").remove();
    recorder->setEncodingSettings(settings, QVideoEncoderSettings(), "wav");
    recorder->setOutputLocation(QUrl::fromLocalFile(QDir::homePath() + "/.thewaveSpeech.wav"));

    testRecorder = new QAudioRecorder(this);
    testRecorder->setEncodingSettings(settings, QVideoEncoderSettings(), "wav");
    testRecorder->setOutputLocation(QUrl::fromLocalFile(QDir::homePath() + "/.thewaveBeforeSpeech.wav"));
}

theWaveWorker::~theWaveWorker() {
    this->stopEverything = true;
    emit finished();
}

void theWaveWorker::begin() {
    if (resetOnNextBegin) { //Check if we need to reset frames
        resetOnNextBegin = false; //Reset Frame Reset bit
        emit resetFrames(); //Tell UI to reset frames
    }

    maxLoudnessForSession = 0; //Reset maximum loudness
    isListeningAfterLoudnessChange = false; //Reset if recorder is listening
    //recorder->record(); //Record audio
    testRecorder->record();
    probe->setSource(testRecorder);

    endListenTimer.start(); //Start end listening timer
    startListeningSound->play(); //Play start sound
    emit outputSpeech("Go for it!");
    emit startedListening(); //Tell UI that we're listening
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
    }

    //Set the maximum loudness
    if (maxLoudnessForSession < loudness) {
        maxLoudnessForSession = loudness;
        if (!isListeningAfterLoudnessChange && loudness > 0.1) { //Start actual recording now
            testRecorder->stop();
            isListeningAfterLoudnessChange = true;
            recorder->record();
            probe->setSource(recorder);
        }
    }

    //If loudness has stayed the same for a second and there has been audio input, stop listening.
    if (endListenTimer.elapsed() >= 1000 && maxLoudnessForSession > 0.1) {
        endAndProcess();
    }
}

void theWaveWorker::endAndProcess() {
    recorder->stop();
    testRecorder->stop();
    emit stoppedListening();
    emit loudnessChanged(-1);

    if (recorder->duration() <= 1000 || maxLoudnessForSession < 0.1) { //Don't process if too short or not loud enough
        stopListeningSound->play();
        emit loudnessChanged(-2);
    } else {
        okListeningSound->play();
        emit outputSpeech("Processing...");

        speechProc = new QProcess(this);
        speechProc->setProcessChannelMode(QProcess::MergedChannels);
        connect(speechProc, SIGNAL(readyRead()), this, SLOT(outputAvailable()));
        speechProc->start("pocketsphinx_continuous -infile \"" + QDir::homePath() + "/.thewaveSpeech.wav\"");
        speechProc->waitForStarted();
    }
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
                    speechProc->deleteLater();
                    //delete speechProc;
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
    QStringList words = parse.split(" ");
    if (speech == "") {
        emit outputResponse("That flew past me. Try again.");
        speak("That flew past me. Try again.", voiceFeedback);
    } else {
        switch (this->state) {
        case Idle:
            if (words.contains("hello") || words.contains("hi")) { //Hello
                if (name == "") {
                    emit outputResponse("Hey there! Just tell me what you'd like me to do.");
                    speak("Hey there! Just tell me what you'd like me to do.");
                } else {
                    emit outputResponse("Hello " + name + "! Just tell me what you'd like me to do.");
                    speak("Hello " + name + "! Just tell me what you'd like me to do.");
                }
                emit showHelpFrame();
                resetOnNextBegin = true;
            } else if (parse == "who am i") { //Name Testing
                if (name == "") {
                    emit outputResponse("I'm not quite sure. Tell me in my settings.");
                    speak("I'm not quite sure. Tell me in my settings.");
                } else {
                    emit outputResponse("You're " + name + ", aren't you?");
                    speak("You're " + name + ", aren't you?");
                }
            } else if (parse == "who are you") { //Who are you
                emit outputResponse("Me? I'm theWave. Pleased to meet you.");
                speak("Me? I'm theWave. Pleased to meet you.");
            } else if (parse == "fuck you") { //Fun
                emit outputResponse("Did I do something wrong?");
                speak("Did I do something wrong?");
            } else if (words.contains("call")) { //Place Call
                emit outputResponse("Unfortunately, I can't place a call from this device.");
                speak("Unfortunately, I can't place a call from this device.");
                emit showCallFrame(false);
                resetOnNextBegin = true;
            } else if (words.contains("text") || words.contains("message")) { //Send Text Message
                emit outputResponse("Unfortunately, I can't send text messages from this device. This functionality may come later for IM applications.");
                speak("Unfortunately, I can't send text messages from this device.");
                emit showMessageFrame();
                resetOnNextBegin = true;
            } else if (words.contains("e-mail") || words.contains("email")) { //Send Email
                if (launchAkonadi()) {
                    emit outputResponse("Who are you sending this email to?.");
                    speak("Who are you sending this email to?"); //Pronunciation issues... :P
                }
            } else if (words.contains("timer") || words.contains("countdown") || parse.contains("count down")) { //Set Timer
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
            } else if (words.contains("help") || parse.contains("what can you do")) { //Get Help
                emit outputResponse("I can do some things. Try asking me something from this list.");
                speak("I can do some things. Try asking me something from this list.");
                emit showHelpFrame();
                resetOnNextBegin = true;
            } else if (parse.startsWith("start", Qt::CaseInsensitive) || parse.startsWith("launch", Qt::CaseInsensitive)) { //Launch App
                emit launchApp(parse.remove(0, 6));
            } else if (words.contains("flight")) { //Flight Information
                emit showFlightFrame("Unknown Airline 000");
                emit outputResponse("I can't find flight information at the moment.");
                speak("I can't find flight information at the moment.");
                resetOnNextBegin = true;
            } else if (words.contains("turn") || words.contains("switch") || words.contains("on") || words.contains("off")) { //Settings
                bool isOn;
                if (words.contains("on")) {
                    isOn = true;
                } else if (words.contains("off")) {
                    isOn = false;
                } else {
                    emit outputResponse("Not sure what you want me to do with that setting. Try telling me to switch on or off.");
                    speak("Not sure what you want me to do with that setting.");
                    return;
                }
                if (words.contains("wi-fi") || words.contains("wifi")) {
                    this->currentSetting = "wifi";
                    emit showSettingsFrame(QIcon::fromTheme("network-wireless"), "Wi-Fi", isOn);
                    this->SetSetting("wifi", isOn);
                    if (isOn) {
                        emit outputResponse("Wi-Fi has been turned on.");
                        speak("Wi-Fi has been turned on.");
                    } else {
                        emit outputResponse("Wi-Fi has been turned off.");
                        speak("Wi-Fi has been turned off.");
                    }
                } else {
                    if (isOn) {
                        emit outputResponse("Not sure what you want me to turn on. You can try asking me a different way.");
                        speak("Not sure what you want me to turn on.");
                    } else {
                        emit outputResponse("Not sure what you want me to turn off. You can try asking me a different way.");
                        speak("Not sure what you want me to turn off.");
                    }
                }
                resetOnNextBegin = true;
            } else if (parse == "where am i") { //Geolocation
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
            } else if (words.contains("song") || words.contains("playing") || words.contains("play") || words.contains("pause") ||
                       ((words.contains("track") || words.contains("song") || words.contains("playing")) &&
                        (words.contains("next") || words.contains("previous") || words.contains("now") || words.contains("current") || words.contains("what")))) { //Media Player Controls
                if (words.contains("pause")) {
                    MainWin->pause();
                    emit outputResponse("Paused.");
                    speak("Paused");
                } else if (words.contains("play")) {
                    MainWin->play();
                    emit outputResponse("Playing.");
                    speak("Playing");
                } else if (words.contains("next")) {
                    MainWin->nextSong();
                    emit outputResponse("Next Track");
                    speak("Next Track");
                } else if (words.contains("previous")) {
                    MainWin->previousSong();
                    emit outputResponse("Previous Track");
                    speak("Previous Track");
                } else if (words.contains("playing") || words.contains("current") || words.contains("what")) {
                    if (MainWin->isMprisAvailable()) {
                        QString title = MainWin->songName();
                        QString artists = MainWin->songArtist();
                        QString album = MainWin->songAlbum();
                        bool playing = MainWin->isMprisPlaying();
                        QPixmap art = QIcon::fromTheme("audio").pixmap(32, 32);
                        QString response = "Not sure what you want me to do with the music.";
                        if (title != "" && artists != "" && album != "") {
                            response = "Right now, " + title + " by " + artists + " on the album " + album + " is playing.";
                            emit showMediaFrame(art, title, artists + " · " + album, playing);
                        } else if (title != "" && artists != "" && album == "") {
                            response = "Right now, " + title + " by " + artists + " is playing.";
                            emit showMediaFrame(art, title, artists, playing);
                        } else if (title != "" && artists == "" && album != "") {
                            response = "Right now, " + title + " on the album " + album + " is playing.";
                            emit showMediaFrame(art, title, album, playing);
                        } else if (title != "" && artists == "" && album != "") {
                            response = "Right now, " + title + " is playing.";
                            emit showMediaFrame(art, title, MainWin->mprisApp(), playing);
                        } else {
                            response = "I'm not sure what's playing now.";
                            //emit showMediaFrame(art, title, artists + " · " + album);
                        }
                        emit outputResponse(response);
                        speak(response);
                    } else {
                        emit outputResponse("There's no media player open now.");
                        speak("There's no media player open now.");
                    }
                }
                resetOnNextBegin = true;
            } else if ((words.contains("calculate") || words.contains("add") || words.contains("plus") || parse.contains("+") || words.contains("subtract") || words.contains("minus") ||
                        parse.contains("-") || words.contains("times") || words.contains("multiply") || parse.contains("*") ||
                        words.contains("over") || words.contains("divide") || parse.contains("divided by") ||
                        parse.contains("/") || words.contains("exponent") || parse.contains("to the power of") || parse.contains("^") ||
                        words.contains("squared") || words.contains("cubed") || words.contains("factorial"))) {
                QString numbersAndOperations = parse;
                numbersAndOperations.remove("calculate");
                numbersAndOperations.remove("what is");
                numbersAndOperations.remove("what's");
                numbersAndOperations.replace("+", " + ");
                numbersAndOperations.replace("-", " - ");
                numbersAndOperations.replace("*", " * ");
                numbersAndOperations.replace("/", " / ");
                numbersAndOperations.replace("^", " ^ ");
                numbersAndOperations.replace("add", "+");
                numbersAndOperations.replace("plus", "+");
                numbersAndOperations.replace("minus", "-");
                numbersAndOperations.replace("subtract", "-");
                numbersAndOperations.replace("negative", "-");
                numbersAndOperations.replace("times", "*");
                numbersAndOperations.replace("multiply", "*");
                numbersAndOperations.replace("over", "/");
                numbersAndOperations.replace("divided by", "/");
                numbersAndOperations.replace("divide", "/");
                numbersAndOperations.replace("exponent", "^");
                numbersAndOperations.replace("to the power of", "^");
                numbersAndOperations.replace("squared", "^ 2");
                numbersAndOperations.replace("cubed", "^ 3");
                numbersAndOperations.replace("factorial", "!");

                QStringList numberReplacer = numbersAndOperations.split(" ");
                float currentNumber = 0;
                int startIndex = -1;
                bool doingPoints = false;
                for (QString part : numberReplacer) {
                    bool isNum;
                    part.toFloat(&isNum);
                    if (isNum) {
                    } else if (part == "point") {
                        if (startIndex != -1) {
                            numberReplacer.replace(startIndex, QString::number(currentNumber));
                            for (int i = startIndex + 1; i < numberReplacer.indexOf(part); i++) {
                                numberReplacer.replace(i, "");
                            }
                            currentNumber = 0;
                        }
                        numberReplacer.replace(numberReplacer.indexOf("point"), ".");
                        startIndex = numberReplacer.indexOf(part + 1);
                        doingPoints = true;
                    } else if (numberDictionary.keys().contains(part)) {
                        float number = numberDictionary.value(part);
                        if (doingPoints) {
                            currentNumber = QString::number(currentNumber).append(QString::number(number)).toInt();
                        } else if (number >= 100) {
                            currentNumber = currentNumber * number;
                        } else {
                            currentNumber = currentNumber + number;
                        }
                        if (startIndex == -1) {
                            startIndex = numberReplacer.indexOf(part);
                        }
                    } else {
                        if (startIndex != -1) {
                            numberReplacer.replace(startIndex, QString::number(currentNumber));
                            for (int i = startIndex + 1; i < numberReplacer.indexOf(part); i++) {
                                numberReplacer.replace(i, "");
                            }
                            currentNumber = 0;
                            startIndex = -1;
                        }
                        doingPoints = false;
                    }
                }

                if (startIndex != -1) {
                    numberReplacer.replace(startIndex, QString::number(currentNumber));
                    for (int i = startIndex + 1; i < numberReplacer.count(); i++) {
                        numberReplacer.replace(i, "");
                    }
                }

                numberReplacer.removeAll("");
                numbersAndOperations = numberReplacer.join(" ");
                numbersAndOperations.replace(" . ", ".");

                QString displayExpression = numbersAndOperations;
                displayExpression.replace("*", "×");
                displayExpression.replace("/", "÷");
                displayExpression.replace(" ^ 2", "²");
                displayExpression.replace(" ^ 3", "³");
                displayExpression.replace(" ^ - 1", "⁻¹");
                displayExpression.replace(" ^ - 2", "⁻²");
                displayExpression.replace(" ^ - 3", "⁻³");
                displayExpression.replace(" !", "!");
                displayExpression.append(" =");

                bool hasErrorOccurred = false;
                QString errorMessage;
                QStringList expressionToCalculate = numbersAndOperations.split(" ");

                while (expressionToCalculate.contains("!") && hasErrorOccurred == false) { //Calcuate Factorials
                    int index = expressionToCalculate.indexOf("!");
                    float number = expressionToCalculate.at(index - 1).toFloat();
                    /*if (index >= 2) {
                        if (expressionToCalculate.at(index - 2) == "-") {
                            hasErrorOccurred = true;
                            errorMessage = "factorialNeg";
                        }
                    }*/
                    if (qFloor(number) != number || qCeil(number) != number) {
                        hasErrorOccurred = true;
                        errorMessage = "factorialNonInt";
                    } else if (number == 0) {
                        expressionToCalculate.removeAt(index);
                        expressionToCalculate.removeAt(index - 1);
                        expressionToCalculate.insert(index - 1, "0");
                    } else {
                        float result = 1;

                        for (int i = 2; i <= number; i++) {
                            result = result * i;
                        }

                        expressionToCalculate.removeAt(index);
                        expressionToCalculate.removeAt(index - 1);
                        expressionToCalculate.insert(index - 1, QString::number(result));
                    }
                }


                while (expressionToCalculate.contains("^") && hasErrorOccurred == false) {
                    int index = expressionToCalculate.indexOf("^");
                    float base = expressionToCalculate.at(index - 1).toFloat();
                    if (index >= 2) {
                        if (expressionToCalculate.at(index - 2) == "-") {
                            base = base * -1;
                        }
                    }

                    float exponent;
                    if (expressionToCalculate.at(index + 1) == "-") {
                        exponent = expressionToCalculate.at(index + 2).toFloat() * -1;
                    } else {
                        exponent = expressionToCalculate.at(index + 1).toFloat();
                    }

                    if (base == 0 && exponent == 0) {
                        hasErrorOccurred = true;
                        errorMessage = "0^0";
                    } else {
                        float result = qPow(base, exponent);
                        if (exponent < 0) {
                            expressionToCalculate.removeAt(index + 2);
                        }
                        expressionToCalculate.removeAt(index + 1);
                        expressionToCalculate.removeAt(index);
                        expressionToCalculate.removeAt(index - 1);
                        if (base < 0) {
                            expressionToCalculate.removeAt(index - 2);
                            expressionToCalculate.insert(index - 2, QString::number(result));
                        } else {
                            expressionToCalculate.insert(index - 1, QString::number(result));
                        }
                    }
                }

                while ((expressionToCalculate.contains("/") || expressionToCalculate.contains("*")) && hasErrorOccurred == false) {
                    bool doDiv;
                    int divIndex = expressionToCalculate.indexOf("/");
                    int mulIndex = expressionToCalculate.indexOf("*");

                    if (divIndex == -1) {
                        doDiv = false;
                    } else if (mulIndex == -1) {
                        doDiv = true;
                    } else if (mulIndex < divIndex) {
                        doDiv = false;
                    } else {
                        doDiv = true;
                    }

                    if (doDiv) {
                        int index = divIndex;
                        float divisor = expressionToCalculate.at(index - 1).toFloat();
                        if (index >= 2) {
                            if (expressionToCalculate.at(index - 2) == "-") {
                                divisor = divisor * -1;
                            }
                        }

                        float dividend;
                        if (expressionToCalculate.at(index + 1) == "-") {
                            dividend = expressionToCalculate.at(index + 2).toFloat() * -1;
                        } else {
                            dividend = expressionToCalculate.at(index + 1).toFloat();
                        }

                        if (dividend == 0 && divisor == 0) {
                            hasErrorOccurred = true;
                            errorMessage = "0div0";
                        } else if (dividend == 0) {
                            hasErrorOccurred = true;
                            errorMessage = "div0";
                        } else {
                            float result = divisor / dividend;
                            if (dividend < 0) {
                                expressionToCalculate.removeAt(index + 2);
                            }
                            expressionToCalculate.removeAt(index + 1);
                            expressionToCalculate.removeAt(index);
                            expressionToCalculate.removeAt(index - 1);
                            if (divisor < 0) {
                                expressionToCalculate.removeAt(index - 2);
                                expressionToCalculate.insert(index - 2, QString::number(result));
                            } else {
                                expressionToCalculate.insert(index - 1, QString::number(result));
                            }
                        }
                    } else {
                        int index = mulIndex;
                        float multiplicand = expressionToCalculate.at(index - 1).toFloat();
                        if (index >= 2) {
                            if (expressionToCalculate.at(index - 2) == "-") {
                                multiplicand = multiplicand * -1;
                            }
                        }

                        float multiplier;
                        if (expressionToCalculate.at(index + 1) == "-") {
                            multiplier = expressionToCalculate.at(index + 2).toFloat() * -1;
                        } else {
                            multiplier = expressionToCalculate.at(index + 1).toFloat();
                        }

                        float result = multiplicand * multiplier;
                        if (multiplicand < 0) {
                            expressionToCalculate.removeAt(index + 2);
                        }
                        expressionToCalculate.removeAt(index + 1);
                        expressionToCalculate.removeAt(index);
                        expressionToCalculate.removeAt(index - 1);
                        if (multiplier < 0) {
                            expressionToCalculate.removeAt(index - 2);
                            expressionToCalculate.insert(index - 2, QString::number(result));
                        } else {
                            expressionToCalculate.insert(index - 1, QString::number(result));
                        }
                    }
                }

                int ignoreNegativeCount = 0;
                int ignoreNegativeIndex = 0;
                while ((expressionToCalculate.contains("+") || expressionToCalculate.count("-") > ignoreNegativeCount) && hasErrorOccurred == false) {
                    bool doAdd;
                    int addIndex = expressionToCalculate.indexOf("+");
                    int subIndex = expressionToCalculate.indexOf("-", ignoreNegativeIndex);

                    if (addIndex == -1) {
                        doAdd = false;
                    } else if (subIndex == -1) {
                        doAdd = true;
                    } else if (subIndex < addIndex) {
                        doAdd = false;
                    } else {
                        doAdd = true;
                    }

                    if (doAdd) {
                        int index = addIndex;
                        float addend1 = expressionToCalculate.at(index - 1).toFloat();
                        if (index >= 2) {
                            if (expressionToCalculate.at(index - 2) == "-") {
                                addend1 = addend1 * -1;
                            }
                        }

                        float addend2;
                        if (expressionToCalculate.at(index + 1) == "-") {
                            addend2 = expressionToCalculate.at(index + 2).toFloat() * -1;
                        } else {
                            addend2 = expressionToCalculate.at(index + 1).toFloat();
                        }

                        float result = addend1 + addend2;
                        if (addend2 < 0) {
                            expressionToCalculate.removeAt(index + 2);
                        }
                        expressionToCalculate.removeAt(index + 1);
                        expressionToCalculate.removeAt(index);
                        expressionToCalculate.removeAt(index - 1);
                        if (addend1 < 0) {
                            expressionToCalculate.removeAt(index - 2);
                            expressionToCalculate.insert(index - 2, QString::number(result));
                        } else {
                            expressionToCalculate.insert(index - 1, QString::number(result));
                        }
                    } else {
                        int index = subIndex;
                        if (index == 0) {
                            ignoreNegativeCount++;
                            ignoreNegativeIndex = index + 1;
                        } else {
                            bool minuendIsFloat;
                            expressionToCalculate.at(index - 1).toFloat(&minuendIsFloat);
                            if (minuendIsFloat) {
                                float minuend = expressionToCalculate.at(index - 1).toFloat();
                                if (index >= 2) {
                                    if (expressionToCalculate.at(index - 2) == "-") {
                                        minuend = minuend * -1;
                                    }
                                }

                                float subtrahend;
                                if (expressionToCalculate.at(index + 1) == "-") {
                                    subtrahend = expressionToCalculate.at(index + 2).toFloat() * -1;
                                } else {
                                    subtrahend = expressionToCalculate.at(index + 1).toFloat();
                                }

                                float result = minuend - subtrahend;
                                if (subtrahend < 0) {
                                    expressionToCalculate.removeAt(index + 2);
                                }
                                expressionToCalculate.removeAt(index + 1);
                                expressionToCalculate.removeAt(index);
                                expressionToCalculate.removeAt(index - 1);
                                if (minuend < 0) {
                                    expressionToCalculate.removeAt(index - 2);
                                    expressionToCalculate.insert(index - 2, QString::number(result));
                                } else {
                                    expressionToCalculate.insert(index - 1, QString::number(result));
                                }
                            } else {
                                ignoreNegativeCount++;
                                ignoreNegativeIndex = index + 1;
                            }
                        }
                    }
                }

                if (hasErrorOccurred) {
                    if (errorMessage == "0div0") {
                        emit showMathematicsFrame(displayExpression, "indeterminate");
                        if (name == "") {
                            emit outputResponse("You've broken mathematics!");
                            speak("You've broken mathematics!");
                        } else {
                            emit outputResponse("You've broken mathematics, " + name + "!");
                            speak("You've broken mathematics, " + name + "!");
                        }
                    } else if (errorMessage == "div0") {
                        emit showMathematicsFrame(displayExpression, "undefined");
                        emit outputResponse("I can't divide by 0.");
                        speak("I can't divide by 0.");
                    } else if (errorMessage == "0^0") {
                        emit showMathematicsFrame(displayExpression, "undefined");
                        emit outputResponse("I can't raise 0 to the power of 0.");
                        speak("I can't raise 0 to the power of 0.");
                    } else if (errorMessage == "factorialNeg") {
                        emit showMathematicsFrame(displayExpression, "undefined");
                        emit outputResponse("I can't calculate the factorial of a negative number.");
                        speak("I can't calculate the factorial of a negative number.");
                    } else if (errorMessage == "factorialNonInt") {
                        emit showMathematicsFrame(displayExpression, "undefined");
                        emit outputResponse("I can't calculate the factorial of a number that isn't an integer.");
                        speak("I can't calculate the factorial of a number that isn't an integer.");
                    }
                } else {
                    QString speakText = expressionToCalculate.first();
                    speakText.replace("e+", " times 10 to the power of ");
                    speakText.replace("e-", " times 10 to the power of negative ");
                    if (speakText.left(1) == "-") {
                        speakText.replace(0, 1, "negative ");
                    }
                    emit showMathematicsFrame(displayExpression, expressionToCalculate.first());
                    emit outputResponse("");
                    speak(speakText);
                }

                resetOnNextBegin = true;
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
            } else if (parse.contains("happy birthday")) { //Fun
                QString output = "It's not my birthday. But I'm assuming it's yours.";
                if (name != "") {
                    output = "It's not my birthday. But I'm assuming it's yours. Happy Birthday " + name + "!";
                }
                emit outputResponse(output);
                speak(output);
            } else { //Look Online
                emit outputResponse("Looking online for information...");
                speak("Looking online for information...");

                bool isInfoFound = false;

                if (settings.value("thewave/wikipediaSearch", true).toBool()) {
                    QEventLoop eventLoop;

                    QNetworkRequest request;
                    QString term = speech;
                    term.replace(" ", "+");
                    QUrl requestUrl("https://en.wikipedia.org/w/api.php?action=query&titles=" + term + "&format=xml&prop=extracts&redirects=true&exintro=true");
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

                        if (stopEverything) {
                            return;
                        }

                        emit showWikipediaFrame(title, text);
                        emit outputResponse("I found some information. Take a look.");
                        speak("I found some information. Take a look.");
                        resetOnNextBegin = true;
                    }

                }

                if (stopEverything) {
                    return;
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
            if (words.contains("hour") || words.contains("minute") || words.contains("second") ||
                    words.contains("hours") || words.contains("minutes") || words.contains("seconds")) {
                int currentNumber = 0, hour = 0, minute = 0, second = 0;
                for (QString part : words) {
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
            } else if (words.contains("cancel") || words.contains("stop")) {
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

bool theWaveWorker::launchAkonadi() {
    if (!Akonadi::ServerManager::isRunning()) {
        emit outputResponse("Starting Akonadi. Please wait.");
        speak("Starting Akonadi. Please wait.");
        if (!Akonadi::ServerManager::start()) {
            emit outputResponse("Unfortunately, the Akonadi Personal Information Management service didn't start properly. Sorry about that.");
            speak("Unfortunately, the Akonadi Personal Information Management service didn't start properly Sorry about that.");
            return false;
        }
    }
    return true;
}

void theWaveWorker::speak(QString speech, bool restartOnceComplete) {
    while (speechPlaying) {
        QApplication::processEvents();
    }

    if (stopEverything) {
        return;
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
        connect(s, static_cast<void(QProcess::*)(int)>(&QProcess::finished), [=]() {
            speechPlaying = false;
        });
    } else if (settings.value("thewave/ttsEngine").toString() == "festival" && QFile("/usr/bin/festival").exists()) {
        QProcess *s = new QProcess(this);
        s->start("festival --tts");
        s->write(speech.toUtf8());
        s->closeWriteChannel();

        if (restartOnceComplete && !stopEverything) {
            connect(s, SIGNAL(finished(int)), this, SLOT(begin()));
        }
        speechPlaying = false;
    } else {
        QTimer* waitTimer = new QTimer(this);
        waitTimer->setInterval(5000);
        waitTimer->setSingleShot(true);
        connect(waitTimer, SIGNAL(timeout()), waitTimer, SLOT(deleteLater()));
        if (restartOnceComplete && !stopEverything) {
            connect(waitTimer, SIGNAL(timeout()), this, SLOT(begin()));
        }
        waitTimer->start();
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
}

void theWaveWorker::launchAppReply(QString app) {
    emit outputResponse("Launching " + app + ".");
    speak("Launching " + app + ".");

    while (speechPlaying) {
        QApplication::processEvents();
        if (stopEverything) {
            return;
        }
    }

    emit doLaunchApp(app);
}

void theWaveWorker::launchApp_disambiguation(QStringList apps) {\
    emit outputResponse("There's more than one app. Select the one you'd like to open on your screen.");
    speak("There's more than one app. Select the one you'd to open on your screen.");
}

void theWaveWorker::currentSettingChanged(bool isOn) {
    this->SetSetting(currentSetting, isOn);
}

void theWaveWorker::SetSetting(QString setting, bool isOn) {
    if (setting == "wifi") {
        MainWin->getInfoPane()->on_WifiSwitch_toggled(isOn);
    }
}
