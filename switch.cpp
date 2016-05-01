#include "switch.h"

Switch::Switch(QWidget *parent) : QPushButton(parent)
{
    this->setCheckable(true);
    this->setFixedSize(this->sizeHint());
}

void Switch::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    painter.setFont(this->font());
    QFontMetrics metrics(this->font());

    if (this->isChecked()) {
        painter.setBrush(this->palette().brush(QPalette::Active, QPalette::Highlight));
        painter.setPen(this->palette().color(QPalette::Active, QPalette::WindowText));
        painter.drawRect(0, 0, this->width(), this->height());
        painter.fillRect(0, 0, this->width(), this->height(), this->palette().brush(QPalette::Active, QPalette::Highlight));

        painter.setBrush(this->palette().brush(QPalette::Active, QPalette::WindowText));
        painter.drawRect(this->width() - this->height(), 0, this->height(), this->height());


        painter.drawStaticText(metrics.height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText("I"));

    } else {
        painter.setBrush(this->palette().brush(QPalette::Active, QPalette::WindowText));
        painter.drawRect(0, 0, this->height(), this->height());

        painter.setBrush(this->palette().brush(QPalette::WindowText));
        painter.drawStaticText(this->height() + metrics.height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText("O"));

    }

    painter.setPen(this->palette().color(QPalette::Active, QPalette::WindowText));
    painter.setBrush(QBrush(Qt::transparent));
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

    event->accept();
}

QSize Switch::sizeHint() {
    return QSize(45, 22);
}
