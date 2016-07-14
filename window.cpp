#include "window.h"

WmWindow::WmWindow()
{

}

void WmWindow::setTitle(QString title) {
    winTitle = title;
}

QString WmWindow::title() const {
    return winTitle;
}

void WmWindow::setPID(unsigned long id) {
    this->id = id;
}

unsigned long WmWindow::PID() const {
    return id;
}

void WmWindow::setWID(Window wid) {
    this->wid = wid;
}

Window WmWindow::WID() const {
    return this->wid;
}

void WmWindow::setIcon(QIcon icon) {
    this->ic = icon;
}

QIcon WmWindow::icon() const {
    return ic;
}

bool WmWindow::attention() const {
    return attn;
}

void WmWindow::setAttention(bool attention) {
    attn = attention;
}

int WmWindow::desktop() const {
    return dk;
}

void WmWindow::setDesktop(int desktop) {
    dk = desktop;
}

bool WmWindow::isMinimized() const {
    return min;
}

void WmWindow::setMinimized(bool minimized) {
    min = minimized;
}
