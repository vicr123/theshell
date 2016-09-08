#include "app.h"

App::App()
{

}

QString App::name() {
    return appname;
}

void App::setName(QString name) {
    appname = name;
}

QIcon App::icon() {
    return appicon;
}

void App::setIcon(QIcon icon) {
    appicon = icon;
}

QString App::command() {
    return appcommand;
}

void App::setCommand(QString command) {
    appcommand = command;
}

QString App::description() {
    return appdesc;
}

void App::setDescription(QString desc) {
    appdesc = desc;
}

bool App::isPinned() {
    return pin;
}

void App::setPinned(bool pinned) {
    pin = pinned;
}

QString App::desktopEntry() {
    return appfile;
}

void App::setDesktopEntry(QString entry) {
    appfile = entry;
}
