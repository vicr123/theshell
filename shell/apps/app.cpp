/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * *************************************/

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

bool App::invalid() {
    return isInvalid;
}

void App::setActions(QList<App> actions) {
    acts = actions;
}

void App::addAction(App action) {
    acts.append(action);
}

QList<App> App::actions() const {
    return acts;
}

App App::invalidApp() {
    App app;
    app.isInvalid = true;
    return app;
}
