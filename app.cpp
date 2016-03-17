#include "app.h"

App::App(QObject *parent) : QObject(parent)
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
