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

#include "menu.h"
#include "ui_menu.h"

extern void EndSession(EndSessionWait::shutdownType type);
extern float getDPIScaling();
extern MainWindow* MainWin;
extern DbusEvents* DBusEvents;
extern TutorialWindow* TutorialWin;
extern NativeEventFilter* NativeFilter;

Menu::Menu(BTHandsfree* bt, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);

    this->resize(this->width() * getDPIScaling(), this->height());
    //ui->listWidget->setIconSize(QSize(16 * getDPIScaling(), 16 * getDPIScaling()));

    ui->offFrame->setParent(this);
    ui->offFrame->setVisible(false);
    this->layout()->removeWidget(ui->offFrame);
    ui->offFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);
    ui->timerIcon->setPixmap(QIcon::fromTheme("player-time").pixmap(16));
    ui->userIcon->setPixmap(QIcon::fromTheme("system-users").pixmap(16));

    ui->commandLinkButton->setProperty("type", "destructive");
    ui->commandLinkButton_2->setProperty("type", "destructive");

    if (!QApplication::arguments().contains("--debug")) {
        ui->exitButton->setVisible(false);
        ui->fakeEndButton->setVisible(false);
    }

    this->setMouseTracking(true);

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }

    QProcess* fullNameProc = new QProcess(this);
    fullNameProc->start("getent passwd " + name);
    fullNameProc->waitForFinished();
    QString parseName(fullNameProc->readAll());
    delete fullNameProc;

    QStringList nameParts = parseName.split(",").at(0).split(":");
    QString fullname;
    if (nameParts.count() > 4) {
        fullname = nameParts.at(4);
    }
    if (fullname == "") {
        ui->label_2->setText(tr("Hey, %1!").arg(name));
    } else {
        ui->label_2->setText(tr("Hey, %1!").arg(fullname));
    }

    //ui->listWidget->installEventFilter(this);
    ui->appsListView->installEventFilter(this);
    ui->pushButton->installEventFilter(this);
    ui->pushButton_3->installEventFilter(this);
    ui->appsListView->viewport()->installEventFilter(this);
    ui->lineEdit->installEventFilter(this);
    this->installEventFilter(this);


    if (QFile("/etc/scallop-live").exists()) {
        ui->InstallLayout->setVisible(true);
    } else {
        ui->InstallLayout->setVisible(false);
    }

    QString seatPath = QString(qgetenv("XDG_SEAT_PATH"));
    if (seatPath == "") {
        ui->commandLinkButton_4->setEnabled(false);
    } else {
        ui->commandLinkButton_4->setEnabled(true);
    }

    //QPalette listPal = ui->listWidget->palette(); //Set List Palette so active colours aren't greyed out
    //listPal.setBrush(QPalette::Inactive, QPalette::Highlight, listPal.brush(QPalette::Active, QPalette::Highlight));
    //listPal.setBrush(QPalette::Inactive, QPalette::HighlightedText, listPal.brush(QPalette::Active, QPalette::HighlightedText));
    //ui->listWidget->setPalette(listPal);

    //populateAppList();
    AppsListModel* appsListModel = new AppsListModel(bt);

    ui->appsListView->setModel(appsListModel);
    ui->appsListView->setItemDelegate(new AppsDelegate);

    //Watch the applications folder
    QFileSystemWatcher* appsWatcher = new QFileSystemWatcher();
    connect(appsWatcher, SIGNAL(fileChanged(QString)), appsListModel, SLOT(loadData()));
    connect(appsWatcher, SIGNAL(directoryChanged(QString)), appsListModel, SLOT(loadData()));
    appsWatcher->addPath("/usr/share/applications/");

    //ui->appsListView->setFlow(QListView::LeftToRight);
    //ui->appsListView->setResizeMode(QListView::Adjust);
    //ui->appsListView->setGridSize(QSize(128 * getDPIScaling(), 128 * getDPIScaling()));
    //ui->appsListView->setViewMode(QListView::IconMode);

    this->bt = bt;
}

Menu::~Menu()
{
    delete ui;
}

void Menu::show() {
    unsigned long desktop = 0xFFFFFFFF;
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    QDialog::show();
    doCheckForClose = true;

    ui->offFrame->setGeometry(10, this->height(), this->width() - 20, this->height() - 20);

    tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(this->x() + this->width(), this->y(), this->width(), this->height()));
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    animation->start();

    ui->lineEdit->setText("");
    ui->lineEdit->setFocus();
    on_lineEdit_textEdited("");

    ui->appsListView->scrollToTop();

    //Show Tutorial Screen
    TutorialWin->showScreen(TutorialWindow::GatewaySearch);
}

void Menu::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (!this->isActiveWindow()) {
            this->close();
        }
    } else if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        ((AppsListModel*) ui->appsListView->model())->loadData();
    }
}

void Menu::close() {
    tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(this->x() - this->width(), this->y(), this->width(), this->height()));
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    connect(animation, &tPropertyAnimation::finished, [=]() {
        emit menuClosing();
        QDialog::hide();
    });
    animation->start();
    doCheckForClose = false;

    //Hide Tutorial Screen
    TutorialWin->hideScreen(TutorialWindow::GatewaySearch);
}

void Menu::checkForclose() {
    if (doCheckForClose) {
        if (!checkFocus(this->layout())) {
            this->close();
        }
    }
}

bool Menu::checkFocus(QLayout *layout) {
    bool hasFocus = this->isActiveWindow();
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != NULL) {
        if (item->layout() != 0) {
            if (checkFocus(item->layout())) {
                hasFocus = true;
            }
        } else if (item->widget() != 0) {
            if (item->widget()->hasFocus()) {
                hasFocus = true;
            }
        }
    }
    return hasFocus;
}

void Menu::on_pushButton_clicked()
{
    if (settings.value("ui/useFullScreenEndSession", false).toBool()) {
        EndSessionWait* endSession = new EndSessionWait(EndSessionWait::ask);
        endSession->showFullScreen();
        endSession->exec();
    } else {
        bool showWarningPane = false;
        if (MainWin->getInfoPane()->isTimerRunning()) {
            showWarningPane = true;
            ui->timerIcon->setVisible(true);
            ui->timerLabel->setVisible(true);
        } else {
            ui->timerIcon->setVisible(false);
            ui->timerLabel->setVisible(false);
        }

        if (false) {
            showWarningPane = true;
            ui->userIcon->setVisible(true);
            ui->userLabel->setVisible(true);
        } else {
            ui->userIcon->setVisible(false);
            ui->userLabel->setVisible(false);
        }

        if (showWarningPane) {
            ui->shutdownText->setText(tr("Before you power off your PC, you may want to check this."));
            ui->shutdownWarnings->setVisible(true);
        } else {
            ui->shutdownText->setText(tr("You're about to power off your PC. Are you sure?"));
            ui->shutdownWarnings->setVisible(false);
        }

        ui->offFrame->setVisible(true);

        /*tPropertyAnimation* anim = new tPropertyAnimation(ui->offFrame, "geometry");
        //anim->setStartValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
        anim->setStartValue(ui->pushButton->geometry());
        anim->setEndValue(QRect(10, 10, this->width() - 20, this->height() - 20));
        anim->setDuration(500);

        anim->setEasingCurve(QEasingCurve::OutCubic);

        anim->start();

        ui->offFrame->setMask(QRegion(0, 0, 0, 0));
        ui->offFrame->setGeometry(10, 10, this->width() - 20, this->height() - 20);*/

        tVariantAnimation* horiz = new tVariantAnimation(this);
        horiz->setStartValue((float) 0);
        horiz->setEndValue((float) 1);
        horiz->setDuration(100);
        horiz->setEasingCurve(QEasingCurve::OutCubic);
        connect(horiz, &tVariantAnimation::valueChanged, [=](QVariant percentage) {
            QRect subtraction;
            subtraction.setTop(ui->pushButton->mapTo(this, QPoint(0, 0)).y());
            subtraction.setHeight(ui->pushButton->height());
            int leftButton = ui->pushButton->mapTo(this, QPoint(0, 0)).x();
            int middleButton = leftButton + ui->pushButton->width() / 2;

            //Calculate left area
            int leftArea = ((float) (middleButton - 10) * percentage.toFloat());

            //Calculate right area
            int rightArea = ((float) (this->width() - middleButton - 10) * percentage.toFloat());

            subtraction.setLeft(middleButton - leftArea);
            subtraction.setRight(middleButton + rightArea);

            ui->offFrame->setGeometry(subtraction);
            ui->offFrame->repaint();
        });
        connect(horiz, &tVariantAnimation::finished, [=] {
            horiz->deleteLater();

            tVariantAnimation* vert = new tVariantAnimation(this);
            vert->setStartValue((float) 0);
            vert->setEndValue((float) 1);
            vert->setDuration(100);
            vert->setEasingCurve(QEasingCurve::InCubic);
            connect(vert, &tVariantAnimation::valueChanged, [=](QVariant percentage) {
                QRect subtraction;
                subtraction.setLeft(10);
                subtraction.setWidth(this->width() - 20);

                int topButton = ui->pushButton->mapTo(this, QPoint(0, 0)).y();

                //Calculate top area
                int topArea = (float) (topButton - (this->height() - ui->pushButton->geometry().bottom())) * percentage.toFloat();

                //Calculate bottom area
                int bottomArea = (float) (this->height() - (topButton + ui->pushButton->height()) - (this->height() - ui->pushButton->geometry().bottom())) * percentage.toFloat();

                subtraction.setTop(topButton - topArea);
                subtraction.setBottom(topButton + ui->pushButton->height() + bottomArea);

                ui->offFrame->setGeometry(subtraction);
            });
            connect(vert, SIGNAL(finished()), vert, SLOT(deleteLater()));
            vert->start();
        });
        connect(horiz, SIGNAL(finished()), horiz, SLOT(deleteLater()));
        horiz->start();
    }
}

void Menu::on_pushButton_2_clicked()
{
    tPropertyAnimation* anim = new tPropertyAnimation(ui->offFrame, "geometry");
    anim->setStartValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    //anim->setEndValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, ui->pushButton->geometry().bottom(), this->width() - 20, 0));
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(anim, &tPropertyAnimation::finished, [=]() {
        ui->offFrame->setVisible(false);
    });

    anim->start();
}

void Menu::on_commandLinkButton_clicked()
{
    EndSession(EndSessionWait::powerOff);
}

void Menu::on_commandLinkButton_2_clicked()
{
    EndSession(EndSessionWait::reboot);
}

void Menu::on_commandLinkButton_3_clicked()
{
    EndSession(EndSessionWait::logout);
}

void Menu::on_lineEdit_textEdited(const QString &arg1)
{
    ((AppsListModel*) ui->appsListView->model())->search(arg1);
    ui->appsListView->selectionModel()->select(ui->appsListView->model()->index(0, 0), QItemSelectionModel::ClearAndSelect);
}

bool Menu::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::KeyPress && ((QKeyEvent*) event)->key() == Qt::Key_Escape) {
        this->close();
        return true;
    }

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = (QKeyEvent*) event;

        int currentRow;
        if (ui->appsListView->selectionModel()->selectedRows().count() == 0) {
            currentRow = -1;
        } else {
            currentRow = ui->appsListView->selectionModel()->selectedRows().first().row();
        }

        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            if (object != ui->lineEdit) {
                if (object == ui->appsListView || object == this) {
                    on_lineEdit_returnPressed();
                } else {
                    ((QPushButton*) object)->click();
                }
            }
        } else if (e->key() == Qt::Key_Right && ui->appsListView->model()->index(currentRow, 0).data(Qt::UserRole + 3).value<App>().actions().count() > 0) {
            showActionMenuByIndex(ui->appsListView->model()->index(currentRow, 0));
            return true;
        } else if (e->key() == Qt::Key_Down) {
            QModelIndex indexToSelect;
            if (currentRow == pinnedAppsCount - 1 && pinnedAppsCount != 0 && ui->lineEdit->text() == "") {
                indexToSelect = ui->appsListView->model()->index(pinnedAppsCount + 1, 0);
            } else if (currentRow == -1) {
                indexToSelect = ui->appsListView->model()->index(0, 0);
            } else if (currentRow == ui->appsListView->model()->rowCount() - 1) {
                indexToSelect = ui->appsListView->model()->index(0, 0);
            } else {
                indexToSelect = ui->appsListView->model()->index(currentRow + 1, 0);
            }
            ui->appsListView->selectionModel()->select(indexToSelect, QItemSelectionModel::ClearAndSelect);
            ui->appsListView->scrollTo(indexToSelect);
            return true;
        } else if (e->key() == Qt::Key_Up) {
            QModelIndex indexToSelect;
            if (currentRow == pinnedAppsCount + 1 && pinnedAppsCount != 0 && ui->lineEdit->text() == "") {
                indexToSelect = ui->appsListView->model()->index(pinnedAppsCount - 1, 0);
            } else if (currentRow == -1) {
                indexToSelect = ui->appsListView->model()->index(ui->appsListView->model()->rowCount() - 1, 0);
            } else if (currentRow == 0) {
                indexToSelect = ui->appsListView->model()->index(ui->appsListView->model()->rowCount() - 1, 0);
            } else {
                indexToSelect = ui->appsListView->model()->index(currentRow - 1, 0);
            }
            ui->appsListView->selectionModel()->select(indexToSelect, QItemSelectionModel::ClearAndSelect);
            ui->appsListView->scrollTo(indexToSelect);
            return true;
        } else {
           if (object != ui->lineEdit) {
               ui->lineEdit->event(event);
               return true;
           }
        }
        e->ignore();
        return false;
    } else if (event->type() == QEvent::MouseButtonRelease && object == ui->appsListView->viewport()) {
        QMouseEvent *e = (QMouseEvent*) event;
        QModelIndex index = ui->appsListView->indexAt(e->pos());
        if (!index.isValid()) {
            return false;
        }

        QList<App> acts = index.data(Qt::UserRole + 3).value<App>().actions();
        if (acts.count() > 0 && e->pos().x() > ui->appsListView->viewport()->width() - 34 * getDPIScaling()) {
            showActionMenuByIndex(index);
        } else {
            launchAppByIndex(index);
        }
        e->ignore();
        return true;
    } else {
        return QDialog::eventFilter(object, event);
    }
}

void Menu::on_lineEdit_returnPressed()
{
    /*if (ui->listWidget->count() > 0) {
        if (ui->listWidget->selectedItems().count() == 1) {
            on_listWidget_itemClicked(ui->listWidget->selectedItems().first());
        } else {
            ui->listWidget->selectedItems().append(ui->listWidget->item(0));
            on_listWidget_itemClicked(ui->listWidget->item(0));
        }
    }*/

    if (ui->appsListView->model()->rowCount() > 0) {
        if (ui->appsListView->selectionModel()->selectedRows().count() == 1) {
            launchAppByIndex(ui->appsListView->selectionModel()->selectedRows().first());
        } else {
            launchAppByIndex(ui->appsListView->model()->index(0, 0));
        }
    }
}

void Menu::on_pushButton_3_clicked()
{
    QProcess::startDetached("scallop");
    this->close();
}

void Menu::on_commandLinkButton_5_clicked()
{
    EndSession(EndSessionWait::suspend);
    this->close();
}

void Menu::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(this->width() - 1, 0, this->width() - 1, this->height());
    event->accept();
}


void Menu::setGeometry(int x, int y, int w, int h) { //Use wmctrl command because KWin has a problem with moving windows offscreen.
    QDialog::setGeometry(x, y, w, h);
    QProcess::execute("wmctrl -r " + this->windowTitle() + " -e 0," +
                      QString::number(x) + "," + QString::number(y) + "," +
                      QString::number(w) + "," + QString::number(h));
}

void Menu::setGeometry(QRect geometry) {
    this->setGeometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void Menu::on_commandLinkButton_7_clicked()
{
    EndSession(EndSessionWait::hibernate);
    this->close();
}

void Menu::on_commandLinkButton_8_clicked()
{
    EndSession(EndSessionWait::screenOff);
    this->close();
}

void Menu::on_commandLinkButton_6_clicked()
{
    this->close();
    DBusEvents->LockScreen();
}

void Menu::on_commandLinkButton_4_clicked()
{
    this->close();
    DBusEvents->LockScreen();

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DisplayManager", QString(qgetenv("XDG_SEAT_PATH")), "org.freedesktop.DisplayManager.Seat", "SwitchToGreeter");
    QDBusConnection::systemBus().send(message);
}

void Menu::mousePressEvent(QMouseEvent *event) {

}

void Menu::mouseMoveEvent(QMouseEvent *event) {

}

void Menu::mouseReleaseEvent(QMouseEvent *event) {

}

void Menu::on_exitButton_clicked()
{
    QApplication::exit(0);
}

void Menu::reject() {
    this->close();
}

void Menu::on_fakeEndButton_clicked()
{
    EndSession(EndSessionWait::dummy);
}

void Menu::on_helpButton_clicked()
{
    QProcess::startDetached("xdg-open https://vicr123.github.io/theshell/help");
    this->close();
}

void Menu::on_reportBugButton_clicked()
{
    QProcess::startDetached("ts-bugreport");
    this->close();
}

void Menu::launchAppByIndex(const QModelIndex &index)
{
    if (((AppsListModel*) ui->appsListView->model())->launchApp(index)) {
        this->close();
    }
}

void Menu::showActionMenuByIndex(QModelIndex index) {
    QMenu* menu = new QMenu();
    menu->addSection(index.data(Qt::DecorationRole).value<QIcon>(), tr("Actions for \"%1\"").arg(index.data(Qt::DisplayRole).toString()));

    QList<App> acts = index.data(Qt::UserRole + 3).value<App>().actions();
    for (App action : acts) {
        menu->addAction(QIcon::fromTheme("arrow-right"), action.name(), [=] {
            QString command = action.command();
            command.remove("env ");
            QProcess* process = new QProcess();
            QStringList environment = process->environment();
            QStringList commandSpace = command.split(" ");
            for (QString part : commandSpace) {
                if (part.contains("=")) {
                    environment.append(part);
                    commandSpace.removeOne(part);
                }
            }
            commandSpace.removeAll("");
            process->start(commandSpace.join(" "));
            connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
            this->close();
            return true;
        });
    }
    QRect rect = ui->appsListView->visualRect(index);
    menu->exec(ui->appsListView->mapToGlobal(rect.topRight()));
}

void Menu::on_appsListView_customContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = ui->appsListView->indexAt(pos);
    if (index.isValid()) {
        QString desktopEntry = index.data(Qt::UserRole + 2).toString();
        if (desktopEntry != "") {
            QMenu* menu = new QMenu();
            menu->addSection(index.data(Qt::DecorationRole).value<QIcon>(), tr("For \"%1\"").arg(index.data(Qt::DisplayRole).toString()));

            if (index.data(Qt::UserRole + 1).toBool()) {
                menu->addAction(QIcon::fromTheme("bookmark-remove"), "Undock", [=] {
                    settings.beginGroup("gateway");
                    QStringList oldEntries;

                    int count = settings.beginReadArray("pinnedItems");
                    for (int i = 0; i < count; i++) {
                        settings.setArrayIndex(i);
                        oldEntries.append(settings.value("desktopEntry").toString());
                    }
                    settings.endArray();

                    oldEntries.removeAll(desktopEntry);

                    settings.beginWriteArray("pinnedItems");
                    int i = 0;
                    for (QString entry : oldEntries) {
                        settings.setArrayIndex(i);
                        settings.setValue("desktopEntry", entry);
                        i++;
                    }
                    settings.endArray();
                    settings.endGroup();

                    ((AppsListModel*) ui->appsListView->model())->loadData();
                });
            } else {
                menu->addAction(QIcon::fromTheme("bookmark-new"), "Dock", [=] {
                    settings.beginGroup("gateway");
                    QStringList oldEntries;

                    int count = settings.beginReadArray("pinnedItems");
                    for (int i = 0; i < count; i++) {
                        settings.setArrayIndex(i);
                        oldEntries.append(settings.value("desktopEntry").toString());
                    }
                    settings.endArray();

                    oldEntries.append(desktopEntry);

                    settings.beginWriteArray("pinnedItems");
                    int i = 0;
                    for (QString entry : oldEntries) {
                        settings.setArrayIndex(i);
                        settings.setValue("desktopEntry", entry);
                        i++;
                    }
                    settings.endArray();
                    settings.endGroup();

                    ((AppsListModel*) ui->appsListView->model())->loadData();
                });
            }
            menu->exec(ui->appsListView->mapToGlobal(pos));
        }
    }
}
