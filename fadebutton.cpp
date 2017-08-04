#include "fadebutton.h"

FadeButton::FadeButton(QWidget *parent) : QPushButton(parent)
{
    this->setFixedWidth(0);
}

bool FadeButton::fade() {
    return f;
}

void FadeButton::setFade(bool fade) {
    f = fade;
    QPalette pal = this->palette();
    QColor col = pal.color(QPalette::ButtonText);
    if (fade) {
        col.setAlpha(127);
    } else {
        col.setAlpha(255);
    }
    pal.setBrush(QPalette::ButtonText, QBrush(col));
    this->setPalette(pal);
    this->repaint();
}

void FadeButton::setFullText(QString fullText) {
    this->txt = fullText;
    QFontMetrics metrics(this->font());
    this->setText(metrics.elidedText(fullText, Qt::ElideRight, 200));
}

void FadeButton::animateIn() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->width());
    anim->setEndValue(this->sizeHint().width());
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void FadeButton::animateOut() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->width());
    anim->setEndValue(0);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedWidth(value.toInt());
    });
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}
