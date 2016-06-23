#include "window.h"

WmWindow::WmWindow(QObject *parent) : QObject(parent)
{

}

void WmWindow::setTitle(QString title) {
    winTitle = title;
}

QString WmWindow::title() {
    return winTitle;
}

void WmWindow::setPID(unsigned long id) {
    this->id = id;
}

unsigned long WmWindow::PID() {
    return id;
}

void WmWindow::setWID(Window wid) {
    this->wid = wid;
}

Window WmWindow::WID() {
    return this->wid;
}

void WmWindow::setIcon(QIcon icon) {
    this->ic = icon;
}

QIcon WmWindow::icon() {
    return ic;
}

bool WmWindow::attention() {
    return attn;
}

void WmWindow::setAttention(bool attention) {
    attn = attention;
}
