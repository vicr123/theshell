#include "app.h"

App::App()
{

}

QString App::name() const {
    return appname;
}

void App::setName(QString name) {
    appname = name;
}

QIcon App::icon() const {
    return appicon;
}

void App::setIcon(QIcon icon) {
    appicon = icon;
}

QString App::command() const {
    return appcommand;
}

void App::setCommand(QString command) {
    appcommand = command;
}

QString App::description() const {
    return appdesc;
}

void App::setDescription(QString desc) {
    appdesc = desc;
}

bool App::isPinned() const {
    return pin;
}

void App::setPinned(bool pinned) {
    pin = pinned;
}

QString App::desktopEntry() const {
    return appfile;
}

void App::setDesktopEntry(QString entry) {
    appfile = entry;
}
