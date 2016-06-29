#include "fadebutton.h"

FadeButton::FadeButton(QWidget *parent) : QPushButton(parent)
{

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
