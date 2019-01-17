/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
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

void WmWindow::setGeometry(QRect geometry) {
    geo = geometry;
}

QRect WmWindow::geometry() const {
    return geo;
}
