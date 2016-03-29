#include "notificationdialog.h"
#include "ui_notificationdialog.h"

NotificationDialog::NotificationDialog(QString title, QString body, QStringList actions, int id, int timeout, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotificationDialog)
{
    ui->setupUi(this);

    QBrush background = this->palette().background();
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Background, pal.foreground());
    pal.setColor(QPalette::Button, pal.foreground().color());
    pal.setColor(QPalette::ButtonText, background.color());
    pal.setBrush(QPalette::Foreground, background);
    //pal().background().setColor(this->palette().foreground().color());
    //pal().foreground().setColor(background);
    this->setPalette(pal);
    ui->title->setText(title);
    ui->body->setText(body);

    for (int i = 0; i < actions.length(); i = i + 2) {
        QString action = actions.at(i);
        QString readable = actions.at(i + 1);

        QPushButton *button = new QPushButton();
        button->setText(readable);
        button->setPalette(pal);
        connect(button, &QPushButton::clicked, [=]() {
            uint id2 = id;
            QString key = action;
            dbusParent->invokeAction(id2, key);
            this->close(2);
        });
        ui->actionsLayout->addWidget(button);
    }

    this->setAttribute(Qt::WA_ShowWithoutActivating, true);

    this->id = id;
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    //w.setGeometry(screenGeometry.x() - 1, screenGeometry.y(), screenGeometry.width(), w.height());
    this->setGeometry(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));



    ui->label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(24, 24));

    if (timeout == -1) { //Timeout default
        this->timeout = 5000;
    } else if (timeout > 0) { //Timeout given by notification
        this->timeout = timeout;
    } else { //No timeout
        this->timeout = -1;
    }
}

void NotificationDialog::setParams(QString title, QString body) {
    ui->title->setText(title);
    ui->body->setText(body);
}

NotificationDialog::~NotificationDialog()
{
    delete ui;
}

void NotificationDialog::show() {
    QDialog::show();


    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    this->setGeometry(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height());

    //QThread::sleep(1); //Buffer for Window Manager Animations

    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), this->height()));
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();
    connect(anim, &QPropertyAnimation::finished, [=](){
        this->repaint();
    });

    if (timeout != -1) {
        QTimer *t = new QTimer(this);
        t->setInterval(timeout);
        t->setSingleShot(true);
        connect(t, &QTimer::timeout, [=]() {
            this->close(1);
        });
        t->start();
    }
}

void NotificationDialog::on_pushButton_clicked()
{
    this->close(2);
}

void NotificationDialog::close(int reason) {
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

    emit closing(id, reason);
}

void NotificationDialog::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void NotificationDialog::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}
