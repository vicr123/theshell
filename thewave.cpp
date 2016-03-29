#include "thewave.h"
#include "ui_thewave.h"

theWave::theWave(InfoPaneDropdown* infoPane, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::theWave)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->infoPane = infoPane;

    ui->editSpeech->setVisible(false);
    resetFrames();
}

theWave::~theWave()
{
    delete ui;
}

void theWave::show() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    this->setGeometry(screenGeometry.x() - 1, screenGeometry.y() - 90, screenGeometry.width() + 1, 90);
    QDialog::show();

    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), 90));
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();

    QThread *t = new QThread();
    worker = new theWaveWorker();
    worker->moveToThread(t);
    connect(t, SIGNAL(started()), worker, SLOT(begin()));
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
    connect(worker, &theWaveWorker::outputResponse, [=](QString response) {
        ui->labelResponse->setText(response);

    });
    connect(worker, &theWaveWorker::outputSpeech, [=](QString speech) {
        ui->labelSpeech->setText(speech);
    });
    connect(worker, &theWaveWorker::startedListening, [=]() {
        ui->pushButton->setIcon(QIcon::fromTheme("mic-on"));
        isListening = true;
    });
    connect(worker, &theWaveWorker::stoppedListening, [=]() {
        ui->pushButton->setIcon(QIcon::fromTheme("mic-off"));
        isListening = false;
    });
    connect(worker, SIGNAL(showCallFrame()), this, SLOT(showCallFrame()));
    connect(worker, SIGNAL(resetFrames()), this, SLOT(resetFrames()));
    connect(worker, SIGNAL(showMessageFrame()), this, SLOT(showMessageFrame()));
    connect(worker, SIGNAL(setTimer(QTime)), infoPane, SLOT(startTimer(QTime)));
    connect(ui->pushButton, SIGNAL(clicked(bool)), worker, SLOT(begin()));
    /*connect(w, &speechWorker::outputFrame, [=](QFrame *frame) {
        ui->frame->layout()->addWidget(frame);
    });*/

    t->start();

    /*QTimer *timer = new QTimer(this);
    timer->setInterval(500);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkForClose()));
    timer->start();*/
}


void theWave::checkForClose() {
    if (!isListening) {
        if (!checkFocus(this->layout())) {
            this->close();
        }
    }
}

bool theWave::checkFocus(QLayout *layout) {
    bool hasFocus = this->isActiveWindow();
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != NULL) {
        if (item->layout() != 0) {
            if (checkFocus(item->layout())) {
                hasFocus = true;
            }
        } else if (item->widget() != 0) {
            if (item->widget()->hasFocus()) {
                hasFocus = true;
            }
        }
    }
    return hasFocus;
}

void theWave::on_pushButton_2_clicked()
{
    worker->quit();
    this->close();
}

void theWave::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &QPropertyAnimation::finished, [=]() {
        QDialog::close();
        //delete this;
    });
    anim->start();

    //emit closing(id);
}

void theWave::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void theWave::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void theWave::resetFrames() {
    ui->callFrame->setVisible(false);
    ui->messageFrame->setVisible(false);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), 90));
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();
}

void theWave::showCallFrame() {
    resetFrames();
    ui->callFrame->setVisible(true);
}

void theWave::showMessageFrame() {
    resetFrames();
    ui->messageFrame->setVisible(true);
}

void theWave::on_pushButton_clicked()
{
    emit startVoice();
}
