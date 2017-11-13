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

#include "background.h"
#include "ui_background.h"

Background::Background(MainWindow* mainwindow, QRect screenGeometry, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Background)
{
    ui->setupUi(this);
    this->mainwindow = mainwindow;

    screenGeometry.moveTo(0, 0);

    QPixmap background(screenGeometry.size());

    QSettings settings;
    QString backPath = settings.value("desktop/background", "inbuilt:triangles").toString();

    if (backPath.startsWith("inbuilt:")) { //Inbuilt background
        QSvgRenderer renderer(QString(":/backgrounds/" + backPath.split(":").at(1)));
        QPainter painter(&background);
        renderer.render(&painter, background.rect());
    } else {
        background.load(backPath);
        background = background.scaled(screenGeometry.size());
    }

    QGraphicsScene* scene = new QGraphicsScene();

    scene->addPixmap(background);
    scene->setSceneRect(screenGeometry);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setSceneRect(screenGeometry);

}

Background::~Background()
{
    delete ui;
}

void Background::show() {
    Atom DesktopWindowTypeAtom;
    DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1);
    QDialog::show();
}

void Background::on_graphicsView_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu = new QMenu(this);
    menu->addSection(tr("For desktop"));
    menu->addAction(ui->actionChange_Background);
    menu->addSection(tr("For system"));
    menu->addAction(ui->actionOpen_theShell_Settings);
    menu->addAction(ui->actionOpen_Status_Center);

    menu->exec(ui->graphicsView->mapToGlobal(pos));
}

void Background::on_actionOpen_Status_Center_triggered()
{
    mainwindow->getInfoPane()->show(InfoPaneDropdown::Clock);
}

void Background::on_actionOpen_theShell_Settings_triggered()
{
    mainwindow->getInfoPane()->show(InfoPaneDropdown::Settings);
}

void Background::on_actionOpen_System_Settings_triggered()
{
    QProcess::startDetached("systemsettings5");
}

void Background::on_actionChange_Background_triggered()
{
    ChooseBackground *background = new ChooseBackground();
    connect(background, SIGNAL(reloadBackgrounds()), mainwindow, SIGNAL(reloadBackgrounds()));
    background->show();
}

void Background::reject() {

}
