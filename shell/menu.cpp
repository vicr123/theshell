/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#include <QScroller>

extern void EndSession(EndSessionWait::shutdownType type);
extern float getDPIScaling();
extern MainWindow* MainWin;
extern DbusEvents* DBusEvents;
extern TutorialWindow* TutorialWin;
extern NativeEventFilter* NativeFilter;
extern ScreenRecorder* screenRecorder;

Menu::Menu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    int thisWidth = settings.value("gateway/width", this->width() * getDPIScaling()).toInt();
    if (thisWidth < 100 * getDPIScaling()) {
        thisWidth = 100 * getDPIScaling();
    }
    if (thisWidth > screenGeometry.width() - 300 * getDPIScaling()) {
        thisWidth = screenGeometry.width() - 300 * getDPIScaling();
    }
    this->resize(thisWidth, this->height());

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

    ui->availableUsersList->setItemDelegate(new AvailableNetworksListDelegate);

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
    AppsListModel* appsListModel = new AppsListModel();

    ui->appsListView->setModel(appsListModel);
    ui->appsListView->setItemDelegate(new AppsDelegate);

    //Watch the applications folder
    QFileSystemWatcher* appsWatcher = new QFileSystemWatcher();
    connect(appsWatcher, SIGNAL(fileChanged(QString)), appsListModel, SLOT(loadData()));
    connect(appsWatcher, SIGNAL(directoryChanged(QString)), appsListModel, SLOT(loadData()));
    appsWatcher->addPath("/usr/share/applications/");

    QScroller::grabGesture(ui->appsListView, QScroller::LeftMouseButtonGesture);

    //ui->appsListView->setFlow(QListView::LeftToRight);
    //ui->appsListView->setResizeMode(QListView::Adjust);
    //ui->appsListView->setGridSize(QSize(128 * getDPIScaling(), 128 * getDPIScaling()));
    //ui->appsListView->setViewMode(QListView::IconMode);

    //this->bt = bt;
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

    ui->stackedWidget->setCurrentIndex(0);

    tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
    animation->setStartValue(this->geometry());

    if (QApplication::isRightToLeft()) {
        animation->setEndValue(QRect(this->x() - this->width(), this->y(), this->width(), this->height()));
    } else {
        animation->setEndValue(QRect(this->x() + this->width(), this->y(), this->width(), this->height()));
    }
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

    if (QApplication::isRightToLeft()) {
        animation->setEndValue(QRect(this->x() + this->width(), this->y(), this->width(), this->height()));
    } else {
        animation->setEndValue(QRect(this->x() - this->width(), this->y(), this->width(), this->height()));
    }
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
        ui->stackedWidget->setCurrentIndex(1);

        ui->endSessionWarnings->clear();
        if (MainWin->getInfoPane()->isTimerRunning()) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText("A timer is currently running");
            item->setIcon(QIcon::fromTheme("chronometer"));
            ui->endSessionWarnings->addItem(item);
        }

        if (screenRecorder->recording()) {
            QListWidgetItem* item = new QListWidgetItem;
            item->setText("You are currently recording your screen");
            item->setIcon(QIcon::fromTheme("media-record"));
            ui->endSessionWarnings->addItem(item);
        }
    }
}

void Menu::on_pushButton_2_clicked()
{
    /*tPropertyAnimation* anim = new tPropertyAnimation(ui->offFrame, "geometry");
    anim->setStartValue(QRect(10 * getDPIScaling(), 10 * getDPIScaling(), this->width() - 20 * getDPIScaling(), this->height() - 20 * getDPIScaling()));
    //anim->setEndValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(this->width(), 10 * getDPIScaling(), this->width() - 20 * getDPIScaling(), this->height() - 20 * getDPIScaling()));
    anim->setDuration(250);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(anim, &tPropertyAnimation::finished, [=]() {
        ui->offFrame->setVisible(false);
    });

    anim->start();*/

    ui->stackedWidget->setCurrentIndex(0);
}

void Menu::on_commandLinkButton_clicked()
{
    if (ui->endSessionWarnings->count() > 0) {
        pendingEndSessionType = EndSessionWait::powerOff;
        ui->endSessionAnywayButton->setText(tr("Power Off Anyway"));
        ui->endSessionAnywayButton->setIcon(QIcon::fromTheme("system-shutdown"));
        ui->stackedWidget->setCurrentIndex(2);
    } else {
        EndSession(EndSessionWait::powerOff);
    }
}

void Menu::on_commandLinkButton_2_clicked()
{
    if (ui->endSessionWarnings->count() > 0) {
        pendingEndSessionType = EndSessionWait::reboot;
        ui->endSessionAnywayButton->setText(tr("Reboot Anyway"));
        ui->endSessionAnywayButton->setIcon(QIcon::fromTheme("system-reboot"));
        ui->stackedWidget->setCurrentIndex(2);
    } else {
        EndSession(EndSessionWait::reboot);
    }
}

void Menu::on_commandLinkButton_3_clicked()
{
    if (ui->endSessionWarnings->count() > 0) {
        pendingEndSessionType = EndSessionWait::logout;
        ui->endSessionAnywayButton->setText(tr("Log Out Anyway"));
        ui->endSessionAnywayButton->setIcon(QIcon::fromTheme("system-log-out"));
        ui->stackedWidget->setCurrentIndex(2);
    } else {
        EndSession(EndSessionWait::logout);
    }
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
        } else if ((QApplication::layoutDirection() == Qt::RightToLeft
                   ? e->key() == Qt::Key_Left
                   : e->key() == Qt::Key_Right) && ui->appsListView->model()->index(currentRow, 0).data(Qt::UserRole + 3).value<App>().actions().count() > 0) {
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
        if (acts.count() > 0 &&
                QApplication::layoutDirection() == Qt::RightToLeft
                ?(e->pos().x() < 34 * getDPIScaling())
                :(e->pos().x() > ui->appsListView->viewport()->width() - 34 * getDPIScaling())) {
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
    //painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(this->palette().color(QPalette::WindowText));
    if (QApplication::isRightToLeft()) {
        painter.drawLine(0, 0, 0, this->height());
    } else {
        painter.drawLine(this->width() - 1, 0, this->width() - 1, this->height());
    }

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

struct LoginSession {
    QString sessionId;
    uint userId;
    QString username;
    QString seat;
    QDBusObjectPath path;
};
Q_DECLARE_METATYPE(LoginSession)

const QDBusArgument &operator<<(QDBusArgument &argument, const LoginSession &session) {
    argument.beginStructure();
    argument << session.sessionId << session.userId << session.username << session.seat << session.path;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LoginSession &session) {
    argument.beginStructure();
    argument >> session.sessionId >> session.userId >> session.username >> session.seat >> session.path;
    argument.endStructure();
    return argument;
}

void Menu::on_commandLinkButton_4_clicked()
{
    //Load available users
    QDBusInterface i("org.freedesktop.login1", "/org/freedesktop/login1/session/self", "org.freedesktop.login1.Session", QDBusConnection::systemBus());
    QString thisId = i.property("Id").toString();

    QDBusMessage availableSessions = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "ListSessions");
    QDBusMessage availableSessionsReply = QDBusConnection::systemBus().call(availableSessions);

    QDBusArgument availableSessionsArg = availableSessionsReply.arguments().first().value<QDBusArgument>();
    QList<LoginSession> sessions;
    availableSessionsArg >> sessions;

    ui->availableUsersList->clear();
    for (LoginSession session : sessions) {
        if (session.sessionId != thisId) {
            QListWidgetItem* item = new QListWidgetItem();

            QDBusInterface i("org.freedesktop.login1", session.path.path(), "org.freedesktop.login1.Session", QDBusConnection::systemBus());
            QString type = i.property("Type").toString();

            QString cls = i.property("Class").toString();
            if (cls == "user") {
                QDBusMessage accountsMessage = QDBusMessage::createMethodCall("org.freedesktop.Accounts", "/org/freedesktop/Accounts", "org.freedesktop.Accounts", "FindUserById");
                accountsMessage.setArguments(QList<QVariant>() << (qlonglong) session.userId);
                QDBusMessage accountsMessageReply = QDBusConnection::systemBus().call(accountsMessage);

                QDBusObjectPath accountObjectPath = accountsMessageReply.arguments().first().value<QDBusObjectPath>();
                QDBusInterface userInterface("org.freedesktop.Accounts", accountObjectPath.path(), "org.freedesktop.Accounts.User", QDBusConnection::systemBus());

                QString name = userInterface.property("RealName").toString();
                if (name == "") {
                    name = session.username;
                }
                item->setText(name);
                item->setIcon(QIcon::fromTheme("user"));

                QString desktop = i.property("Desktop").toString();

                QString secondLine;
                if (type == "x11") {
                    secondLine = tr("%1 on X11 display %2").arg(desktop, i.property("Display").toString());
                } else if (type == "tty") {
                    secondLine = tr("on %1").arg(i.property("TTY").toString());
                } else if (type == "wayland") {
                    secondLine = tr("%1 on VT #%2").arg(desktop, QString::number(i.property("VTNr").toUInt()));
                } else {
                    secondLine = tr("Session");
                }
                item->setData(Qt::UserRole + 1, secondLine);
            } else if (cls == "greeter") {
                item->setText(session.username);
                item->setIcon(QIcon::fromTheme("arrow-right"));
                item->setData(Qt::UserRole + 1, "Login Screen");
            } else {
                delete item;
                continue;
            }
            item->setData(Qt::UserRole, session.path.path());

            ui->availableUsersList->addItem(item);
        }
    }

    if (ui->availableUsersList->count() == 0) {
        ui->startNewSessionButton->click();
    } else {
        ui->stackedWidget->setCurrentIndex(3);
    }
}

void Menu::mousePressEvent(QMouseEvent *event) {
    if (QApplication::isRightToLeft()) {
        if (event->x() < 5) {
            resizing = true;
        }
    } else {
        if (event->x() > this->width() - 5) {
            resizing = true;
        }
    }
}

void Menu::mouseMoveEvent(QMouseEvent *event) {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (resizing) {
        int width;
        if (QApplication::isRightToLeft()) {
            width = screenGeometry.right() - QCursor::pos().x();
        } else {
            width = event->x();
        }
        if (width < 100 * getDPIScaling()) {
            width = 100 * getDPIScaling();
        }
        if (width > screenGeometry.width() - 300 * getDPIScaling()) {
            width = screenGeometry.width() - 300 * getDPIScaling();
        }
        this->resize(width, this->height());
        if (QApplication::isRightToLeft()) {
            this->move(screenGeometry.right() - width, this->y());
        }
    } else {
        if (QApplication::isRightToLeft()) {
            if (event->x() < 5) {
                this->setCursor(QCursor(Qt::SizeHorCursor));
            } else {
                this->setCursor(QCursor(Qt::ArrowCursor));
            }
        } else {
            if (event->x() > this->width() - 5) {
                this->setCursor(QCursor(Qt::SizeHorCursor));
            } else {
                this->setCursor(QCursor(Qt::ArrowCursor));
            }
        }
    }
}

void Menu::mouseReleaseEvent(QMouseEvent *event) {
    resizing = false;
    this->setCursor(QCursor(Qt::ArrowCursor));
    settings.setValue("gateway/width", this->width());
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
    if (ui->endSessionWarnings->count() > 0) {
        pendingEndSessionType = EndSessionWait::dummy;
        ui->endSessionAnywayButton->setText("Perform Fake Exit Anyway");
        ui->endSessionAnywayButton->setIcon(QIcon::fromTheme("arrow-right"));
        ui->stackedWidget->setCurrentIndex(2);
    } else {
        EndSession(EndSessionWait::dummy);
    }
}

void Menu::on_helpButton_clicked()
{
    QProcess::startDetached("xdg-open https://vicr123.github.io/theshell/help");
    this->close();
}

void Menu::on_reportBugButton_clicked()
{
    QProcess::startDetached("xdg-open https://github.com/vicr123/theshell/issues");
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

    menu->exec(ui->appsListView->mapToGlobal(QApplication::layoutDirection() == Qt::RightToLeft
                                             ? rect.topLeft()
                                             : rect.topRight()));
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

void Menu::on_cancelEndSessionWarningButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Menu::on_endSessionAnywayButton_clicked()
{
    EndSession(pendingEndSessionType);
}

void Menu::on_cancelSwitchUsersButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void Menu::on_startNewSessionButton_clicked()
{
    this->close();
    DBusEvents->LockScreen();

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DisplayManager", QString(qgetenv("XDG_SEAT_PATH")), "org.freedesktop.DisplayManager.Seat", "SwitchToGreeter");
    QDBusConnection::systemBus().send(message);
}

void Menu::on_availableUsersList_itemActivated(QListWidgetItem *item)
{
    this->close();
    DBusEvents->LockScreen();

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", item->data(Qt::UserRole).toString(), "org.freedesktop.login1.Session", "Activate");
    QDBusConnection::systemBus().send(message);
}
