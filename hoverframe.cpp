#include "hoverframe.h"

HoverFrame::HoverFrame(QWidget *parent) : QFrame(parent)
{
    this->setMouseTracking(true);
}

void HoverFrame::enterEvent(QEvent *event) {
    emit MouseEnter();
}

void HoverFrame::leaveEvent(QEvent *event) {
    emit MouseExit();
}
