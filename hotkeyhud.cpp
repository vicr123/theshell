#include "hotkeyhud.h"
#include "ui_hotkeyhud.h"

HotkeyHud::HotkeyHud(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HotkeyHud)
{
    ui->setupUi(this);
}

HotkeyHud::~HotkeyHud()
{
    delete ui;
}

void HotkeyHud::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void HotkeyHud::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void HotkeyHud::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
    event->accept();
}

void HotkeyHud::show() {
    QDialog::show();

    if (!isShowing) {
        QRect screenGeometry = QApplication::desktop()->screenGeometry();
        this->setGeometry(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height());

        QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
        anim->setStartValue(this->geometry());
        anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y(), screenGeometry.width(), this->height()));
        anim->setDuration(100);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start();
        connect(anim, &QPropertyAnimation::finished, [=](){
            this->repaint();
        });
    }

    if (timeout == NULL) {
        timeout = new QTimer();
        timeout->setSingleShot(true);
        timeout->setInterval(5000);
        connect(timeout, SIGNAL(timeout()), this, SLOT(Timeout()));

    }
    timeout->start();

    isShowing = true;
}

void HotkeyHud::show(QIcon icon, QString control, int value) {
    ui->icon->setPixmap(icon.pixmap(32));
    ui->control->setText(control);
    ui->slider->setValue(value);
    ui->value->setText(QString::number(value) + "%");
    ui->explanation->setVisible(false);
    ui->slider->setVisible(true);
    ui->value->setVisible(true);
    this->show();
}

void HotkeyHud::show(QIcon icon, QString control, QString explanation) {
    ui->icon->setPixmap(icon.pixmap(32));
    ui->control->setText(control);
    ui->explanation->setText(explanation);
    ui->slider->setVisible(false);
    ui->value->setVisible(false);
    ui->explanation->setVisible(true);
    this->show();
}

void HotkeyHud::Timeout() {
    timeout->deleteLater();
    timeout = NULL;
    this->close();
}

void HotkeyHud::close() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    QPropertyAnimation *anim = new QPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(screenGeometry.x(), screenGeometry.y() - this->height(), screenGeometry.width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &QPropertyAnimation::finished, [=]() {
        QDialog::close();
        isShowing = false;
    });
    anim->start();
}
