#include "notificationdialog.h"
#include "ui_notificationdialog.h"

NotificationDialog::NotificationDialog(QString title, QString body, int id, int timeout, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotificationDialog)
{
    ui->setupUi(this);

    ui->title->setText(title);
    ui->body->setText(body);

    this->setAttribute(Qt::WA_ShowWithoutActivating, true);

    this->id = id;
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    //w.setGeometry(screenGeometry.x() - 1, screenGeometry.y(), screenGeometry.width(), w.height());
    this->setGeometry(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));


    QBrush background = this->palette().background();
    QPalette pal = this->palette();
    pal.setBrush(QPalette::Background, pal.foreground());
    pal.setBrush(QPalette::Foreground, background);
    //pal().background().setColor(this->palette().foreground().color());
    //pal().foreground().setColor(background);
    this->setPalette(pal);

    ui->label->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(24, 24));

    if (timeout == -1) {
        this->timeout = 5000;
    } else if (timeout > 0) {
        this->timeout = timeout;
    } else {
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
    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), this->height()));
    anim->setDuration(100);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();

    if (timeout != -1) {
        QTimer *t = new QTimer(this);
        t->setInterval(timeout);
        t->setSingleShot(true);
        connect(t, &QTimer::timeout, [=]() {
            this->close();
        });
        t->start();
    }
}

void NotificationDialog::on_pushButton_clicked()
{
    this->close();
}

void NotificationDialog::close() {
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
