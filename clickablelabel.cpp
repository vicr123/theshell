#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget* parent) :
    QLabel(parent)
{

}

void ClickableLabel::mousePressEvent(QMouseEvent *event) {
    isClicked = true;
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *event) {
    if (didDrag) {
        emit mouseReleased();
        didDrag = false;
    }
    if (event->x() < this->width() && event->y() < this->height()) {
        emit clicked();
    }
    isClicked = false;
    event->accept();
}

bool ClickableLabel::showDisabled() {
    return isShowDisabled;
}

void ClickableLabel::setShowDisabled(bool showDisabled) {
    isShowDisabled = showDisabled;
    QLabel* tempLabel = new QLabel;
    QPalette applicationPal = QApplication::palette(tempLabel);
    QPalette thisPal = this->palette();
    if (isShowDisabled) {
        //thisPal.setCurrentColorGroup(QPalette::Disabled);
        thisPal.setBrush(QPalette::WindowText, applicationPal.brush(QPalette::Disabled, QPalette::WindowText));
    } else {
        //thisPal.setCurrentColorGroup(QPalette::Active);
        thisPal.setBrush(QPalette::WindowText, applicationPal.brush(QPalette::Normal, QPalette::WindowText));
    }
    this->setPalette(thisPal);
    delete tempLabel;
}

void ClickableLabel::mouseMoveEvent(QMouseEvent *event) {
    if (isClicked) {
        emit dragging(event->x(), event->y());
        didDrag = true;
    }
}
