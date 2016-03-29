#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent) :
    QLabel(parent)
{

}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
    emit clicked();
    event->accept();
}

bool ClickableLabel::showDisabled() {
    return isShowDisabled;
}

void ClickableLabel::setShowDisabled(bool showDisabled) {
    isShowDisabled = showDisabled;
    QPalette applicationPal = QApplication::palette(new QLabel());
    QPalette thisPal = this->palette();
    if (isShowDisabled) {
        //thisPal.setCurrentColorGroup(QPalette::Disabled);
        thisPal.setBrush(QPalette::WindowText, applicationPal.brush(QPalette::Disabled, QPalette::WindowText));
    } else {
        //thisPal.setCurrentColorGroup(QPalette::Active);
        thisPal.setBrush(QPalette::WindowText, applicationPal.brush(QPalette::Normal, QPalette::WindowText));
    }
    this->setPalette(thisPal);
}
