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

void WmWindow::setPID(int id) {
    this->id = id;
}

int WmWindow::PID() {
    return id;
}
