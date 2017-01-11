#include "switch.h"

Switch::Switch(QWidget *parent) : QPushButton(parent)
{
    this->setCheckable(true);
    this->setFixedSize(this->sizeHint());

    innerRect = QRect(0, 0, this->height(), this->width());

    connect(this, SIGNAL(toggled(bool)), this, SLOT(checkChanging(bool)));
}

QPalette::ColorGroup Switch::IsActiveColorRole() {
    if (this->isEnabled()) {
        return QPalette::Active;
    } else {
        return QPalette::Disabled;
    }
}

void Switch::paintEvent(QPaintEvent *event) {
    QPainter painter(this);

    painter.setFont(this->font());
    QFontMetrics metrics(this->font());

    painter.setBrush(this->palette().brush(IsActiveColorRole(), QPalette::Highlight));
    painter.setPen(this->palette().color(IsActiveColorRole(), QPalette::WindowText));
    painter.drawRect(0, 0, innerRect.left(), this->height());
    //painter.fillRect(0, 0, this->width(), this->height(), this->palette().brush(QPalette::Active, QPalette::Highlight));

    painter.setBrush(this->palette().brush(IsActiveColorRole(), QPalette::HighlightedText));
    painter.drawRect(innerRect);

    //painter.drawStaticText(metrics.height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText("I"));

    if (iText == "") {
        painter.drawPixmap(innerRect.left() - 10 - this->height() / 2, (this->height() / 2 - 16 / 2), 16, 16, this->iIcon.pixmap(16, 16));

    } else {
        painter.drawStaticText(innerRect.left() - metrics.width(iText) - this->height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText(iText));
    }
    painter.setBrush(this->palette().brush(IsActiveColorRole(), QPalette::WindowText));
    //painter.drawStaticText(this->height() + metrics.height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText("O"));
    painter.drawStaticText(innerRect.right() + this->height() / 2, (this->height() / 2 - metrics.height() / 2), QStaticText(oText));

    painter.setPen(this->palette().color(IsActiveColorRole(), QPalette::WindowText));
    painter.setBrush(QBrush(Qt::transparent));
    painter.drawRect(0, 0, this->width() - 1, this->height() - 1);

    event->accept();
}

void Switch::mousePressEvent(QMouseEvent *event) {
    mouseClickPoint = event->localPos().toPoint().x();
    initialPoint = mouseClickPoint;
}

void Switch::mouseMoveEvent(QMouseEvent *event) {
    if (event->localPos().toPoint().x() < mouseClickPoint) {
        mouseMovedLeft = true;
    } else {
        mouseMovedLeft = false;
    }

    innerRect.translate(event->localPos().toPoint().x() - mouseClickPoint, 0);
    if (innerRect.x() <= 0) {
        innerRect.moveTo(0, 0);
    } else if (innerRect.right() >= this->width()) {
        innerRect.moveTo(this->width() - innerRect.width(), 0);
    }

    mouseClickPoint = event->localPos().toPoint().x();
    this->repaint();
}

void Switch::mouseReleaseEvent(QMouseEvent *event) {
    if (initialPoint - 2 < mouseClickPoint && initialPoint + 2 > mouseClickPoint) {

        this->setChecked(!this->isChecked());
        this->checkChanging(!this->isChecked());
    } else {
        if (mouseMovedLeft) {
            this->setChecked(false);
            this->checkChanging(false);
        } else {
            this->setChecked(true);
            this->checkChanging(true);
        }
    }
}

void Switch::checkChanging(bool checked) {
    tVariantAnimation* animation = new tVariantAnimation();
    animation->setStartValue(innerRect);
    animation->setDuration(250);
    animation->setForceAnimation(true);

    if (checked) {
        animation->setEndValue(QRect(this->width() - this->height(), 0, this->height(), this->height()));
    } else {
        animation->setEndValue(QRect(0, 0, this->height(), this->height()));
    }

    animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(animation, &tVariantAnimation::valueChanged, [=](QVariant value) {
        innerRect = value.toRect();
        this->repaint();
    });
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    animation->start();
}

QSize Switch::sizeHint() const {
    QFontMetrics metrics(this->font());
    int width = 33;
    if (iText == "") {
        width += 8;
    } else {
        width += metrics.width(iText);
    }

    width += metrics.width(oText);
    return QSize(width, 22);
}

QString Switch::OnText() {
    return iText;
}

QString Switch::OffText() {
    return oText;
}

void Switch::setOnText(QString text) {
    iText = text;
    this->iIcon = QIcon();
    this->setFixedSize(this->sizeHint());
    this->repaint();
}

void Switch::setOffText(QString text) {
    oText = text;
    this->setFixedSize(this->sizeHint());
    this->repaint();
}

void Switch::setOnIcon(QIcon icon) {
    this->iIcon = icon;
    iText = "";
    this->setFixedSize(this->sizeHint());
    this->repaint();
}

QIcon Switch::OnIcon() {
    return this->iIcon;
}
