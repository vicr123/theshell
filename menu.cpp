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
    ui->thewaveFrame->setParent(this);
    this->layout()->removeWidget(ui->offFrame);
    this->layout()->removeWidget(ui->thewaveFrame);
    ui->offFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);
    ui->thewaveFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);
    ui->timerIcon->setPixmap(QIcon::fromTheme("player-time").pixmap(16));
    ui->userIcon->setPixmap(QIcon::fromTheme("system-users").pixmap(16));
    ui->thewave_connection_disconnection_label->setPixmap(QIcon::fromTheme("network-disconnect").pixmap(16));
    ui->timeIcon->setPixmap(QIcon::fromTheme("player-time").pixmap(32));
    ui->callIcon->setPixmap(QIcon::fromTheme("call-start").pixmap(32));
    ui->messageIcon->setPixmap(QIcon::fromTheme("message-send").pixmap(32));
    ui->launchIcon->setPixmap(QIcon::fromTheme("system-run").pixmap(32));
    ui->infoIcon->setPixmap(QIcon::fromTheme("text-html").pixmap(32));
    ui->mathIcon->setPixmap(QIcon::fromTheme("accessories-calculator").pixmap(32));
    ui->settingsIcon->setPixmap(QIcon::fromTheme("preferences-system").pixmap(32));
    ui->mediaIcon->setPixmap(QIcon::fromTheme("media-playback-start").pixmap(32));

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
    ui->activateTheWave->installEventFilter(this);
    ui->pushButton->installEventFilter(this);
    ui->pushButton_3->installEventFilter(this);
    this->installEventFilter(this);


    if (QFile("/usr/bin/install_theos").exists()) {
        ui->InstallLayout->setVisible(true);
    } else {
        ui->InstallLayout->setVisible(false);
    }

    QNetworkAccessManager networkManager;
    if (networkManager.networkAccessible() == QNetworkAccessManager::Accessible) {
        ui->thewaveInternetFrame->setVisible(false);
    } else {
        ui->thewaveInternetFrame->setVisible(true);
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

    this->theWaveFrame = ui->thewaveFrame;

    //populateAppList();
    AppsListModel* appsListModel = new AppsListModel(bt);
    connect(appsListModel, &AppsListModel::queryWave, [=](QString query) {
        ui->activateTheWave->click();
        if (query != "") {
            ui->thewave_line->setText(query);
            on_thewave_line_returnPressed();
        }
    });

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

void Menu::show(bool openTotheWave, bool startListening) {
    if (this->isVisible()) {
        this->uncollapse();
    } else {
        //Reload menu
        //((AppsListModel*) ui->appsListView->model())->loadData();

        unsigned long desktop = 0xFFFFFFFF;
        XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                         XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

        QDialog::show();
        doCheckForClose = true;

        ui->offFrame->setGeometry(10, this->height(), this->width() - 20, this->height() - 20);
        ui->thewaveFrame->setGeometry(10, this->height(), this->width() - 20, this->height() - 20);

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

    if (openTotheWave && !this->istheWaveOpen) {
        ui->activateTheWave->click();
    }
    if (startListening && openTotheWave) {
        ui->listentheWave->click();
    }

    connect(NativeFilter, &NativeEventFilter::DoRetranslation, [=] {
        ui->retranslateUi(this);
    });
}

void Menu::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (istheWaveOpen) {
            if (this->isActiveWindow()) {
                uncollapse();
            } else {
                collapse();
            }
        } else {
            if (!this->isActiveWindow()) {
                this->close();
            }
        }
    }
}

void Menu::collapse() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(screenGeometry.x() - (this->width() - 50), this->y(), this->width(), this->height()));
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start();
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

}

void Menu::uncollapse() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    tPropertyAnimation* animation = new tPropertyAnimation(this, "geometry");
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(screenGeometry.x(), this->y(), this->width(), this->height()));
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start();
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
}

void Menu::close() {
    if (istheWaveOpen) {
        ui->closetheWaveButton->click();
    }
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
}

bool Menu::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::KeyPress && ((QKeyEvent*) event)->key() == Qt::Key_Escape) {
        this->close();
        return true;
    } else {
        //if (object != ui->thewave_line && object != ui->lineEdit) {
        //if (ui->thewave_line->hasFocus() || ui->lineEdit->hasFocus()) {
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent *e = (QKeyEvent*) event;
                if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
                    if (!istheWaveReady) {
                        if (object != ui->lineEdit && object != ui->thewave_line && object != this) {
                            if (object == ui->appsListView) {
                                on_lineEdit_returnPressed();
                            } else {
                                ((QPushButton*) object)->click();
                            }
                            on_lineEdit_returnPressed();
                        }
                    }
                    event->ignore();
                } else {
                    if (istheWaveReady) {
                        ui->thewave_line->setText(ui->thewave_line->text().append(e->text())); //Type letter into theWave
                        ui->thewave_line->setFocus();
                    } else {
                        if (e->key() != Qt::Key_Up && e->key() != Qt::Key_Down) {
                            ui->lineEdit->setText(ui->lineEdit->text().append(e->text())); //Type letter into search box
                            ui->lineEdit->setFocus();
                            on_lineEdit_textEdited(ui->lineEdit->text());
                        } else {
                            int currentRow;
                            if (ui->appsListView->selectionModel()->selectedRows().count() == 0) {
                                currentRow = -1;
                            } else {
                                currentRow = ui->appsListView->selectionModel()->selectedRows().first().row();
                            }

                            if (e->key() == Qt::Key_Down) {
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
                            }
                        }
                    }
                }
                e->ignore();
                return true;
            } else {
                return QDialog::eventFilter(object, event);
            }
        /*} else {
            if (event->type() == QEvent::KeyPress) {
                event->ignore();
                return true;
            }
            return QDialog::eventFilter(object, event);
        }*/
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
            on_appsListView_clicked(ui->appsListView->selectionModel()->selectedRows().first());
        } else {
            on_appsListView_clicked(ui->appsListView->model()->index(0, 0));
        }
    }
}

void Menu::on_pushButton_3_clicked()
{
    QProcess::startDetached("install_theos");
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
    this->close();
    QProcess::startDetached("xset dpms force off");
}

void Menu::on_activateTheWave_clicked()
{
    istheWaveOpen = true;
    this->resetFrames();
    tPropertyAnimation* anim = new tPropertyAnimation(ui->thewaveFrame, "geometry");
    anim->setStartValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start();

    waveWorker = new theWaveWorker();

    if (waveWorker->isDisabled()) {
        ui->thewaveDisabledFrame->setVisible(true);
        ui->thewaveFrameInner->setVisible(false);
        ui->listentheWave->setVisible(false);
    } else {
        ui->thewaveDisabledFrame->setVisible(false);
        ui->thewaveFrameInner->setVisible(true);
        ui->listentheWave->setVisible(true);
        ui->theWaveSpeakFrame->setVisible(false);
        ui->thewave_hintFrame->setVisible(false);
        ui->thewaveHintIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(16, 16));

        QThread *t = new QThread();
        waveWorker->moveToThread(t);

        //connect(t, SIGNAL(started()), waveWorker, SLOT(begin()));
        connect(t, &QThread::started, [=]() {
           this->istheWaveReady = true;
        });
        connect(waveWorker, SIGNAL(finished()), waveWorker, SLOT(deleteLater()));
        connect(t, SIGNAL(finished()), t, SLOT(deleteLater()));
        connect(waveWorker, &theWaveWorker::outputResponse, [=](QString response) {
            if (settings.value("thewave/blockOffensiveWords").toBool()) {
                //Sorry... If there's a better way though, that'd be good :)
                response.replace("shit", "s•••", Qt::CaseInsensitive);
                response.replace("fuck", "f•••", Qt::CaseInsensitive);
                response.replace("cunt", "c•••", Qt::CaseInsensitive);
                response.replace("bitch", "b••••", Qt::CaseInsensitive);
            }

            ui->thewave_response->setText(response);
            ui->thewave_hintFrame->setVisible(false);
        });
        connect(waveWorker, &theWaveWorker::outputHint, [=](QString hint) {
            ui->thewaveHint->setText(hint);
            ui->thewave_hintFrame->setVisible(true);
        });
        connect(waveWorker, SIGNAL(outputSpeech(QString)), this, SLOT(thewave_outputSpeech(QString)));
        connect(waveWorker, &theWaveWorker::startedListening, [=]() {
            isListening = true;
        });
        connect(waveWorker, &theWaveWorker::stoppedListening, [=]() {
            isListening = false;
        });
        connect(waveWorker, &theWaveWorker::doLaunchApp, [=](QString appName) {
            for (App app : ((AppsListModel*) ui->appsListView->model())->availableApps()) {
                if (app.name() == appName) {
                    QProcess::startDetached(app.command().remove("%u"));
                    this->close();
                }
            }
        });
        connect(waveWorker, &theWaveWorker::showBigListenFrame, [=]() {
            QString name = settings.value("thewave/name").toString();
            if (name == "") {
                switch (qrand() % 5) {
                case 0:
                    ui->theWaveSpeakGreeting->setText("Hey there. What's next?");
                    break;
                case 1:
                    ui->theWaveSpeakGreeting->setText("What can I do for you?");
                    break;
                case 2:
                    ui->theWaveSpeakGreeting->setText("Hello.");
                    break;
                case 3:
                    ui->theWaveSpeakGreeting->setText("Oh, hey there!");
                    break;
                case 4:
                    QTime current = QTime::currentTime();
                    if (current.hour() < 12) {
                        ui->theWaveSpeakGreeting->setText("Good morning!");
                    } else if (current.hour() < 17) {
                        ui->theWaveSpeakGreeting->setText("Good afternoon!");
                    } else {
                        ui->theWaveSpeakGreeting->setText("Good evening!");
                    }
                    break;
                }
            } else {
                switch (qrand() % 5) {
                case 0:
                    ui->theWaveSpeakGreeting->setText("Hey, " + name + ". What's next?");
                    break;
                case 1:
                    ui->theWaveSpeakGreeting->setText("What can I do for you, " + name + "?");
                    break;
                case 2:
                    ui->theWaveSpeakGreeting->setText("Hello " + name + "!");
                    break;
                case 3:
                    ui->theWaveSpeakGreeting->setText("Oh, hey " + name + "!");
                    break;
                case 4:
                    QTime current = QTime::currentTime();
                    if (current.hour() < 12) {
                        ui->theWaveSpeakGreeting->setText("Good morning " + name + "!");
                    } else if (current.hour() < 17) {
                        ui->theWaveSpeakGreeting->setText("Good afternoon " + name + "!");
                    } else {
                        ui->theWaveSpeakGreeting->setText("Good evening " + name + "!");
                    }
                    break;
                }
            }

            ui->thewaveFrameInner->setVisible(false);
            ui->theWaveSpeakFrame->setVisible(true);
        });
        connect(waveWorker, &theWaveWorker::hideBigListenFrame, [=]() {
            ui->thewaveFrameInner->setVisible(true);
            ui->theWaveSpeakFrame->setVisible(false);
        });
        connect(waveWorker, SIGNAL(showCallFrame(bool)), this, SLOT(showCallFrame(bool))); //Call
        connect(waveWorker, SIGNAL(resetFrames()), this, SLOT(resetFrames())); //Reset
        connect(waveWorker, SIGNAL(showMessageFrame()), this, SLOT(showMessageFrame())); //Text Message
        connect(waveWorker, SIGNAL(showHelpFrame()), this, SLOT(showHelpFrame())); //Help
        connect(waveWorker, SIGNAL(showWikipediaFrame(QString,QString)), this, SLOT(showWikipediaFrame(QString,QString))); //Wikipedia
        connect(waveWorker, SIGNAL(launchApp(QString)), this, SLOT(thewave_launchapp(QString))); //Launch
        connect(waveWorker, SIGNAL(setTimer(QTime)), MainWin->getInfoPane(), SLOT(startTimer(QTime))); //Start a timer
        connect(waveWorker, SIGNAL(showFlightFrame(QString)), this, SLOT(showFlightFrame(QString))); //Flight
        connect(waveWorker, SIGNAL(loudnessChanged(qreal)), this, SLOT(thewaveLoudnessChanged(qreal))); //Input Loudness
        connect(waveWorker, SIGNAL(showSettingsFrame(QIcon,QString,bool)), this, SLOT(showSettingFrame(QIcon,QString,bool))); //Settings
        connect(waveWorker, SIGNAL(showMathematicsFrame(QString,QString)), this, SLOT(showMathematicsFrame(QString,QString))); //Mathematics
        connect(waveWorker, SIGNAL(showMediaFrame(QPixmap,QString,QString,bool)), this, SLOT(showMediaFrame(QPixmap,QString,QString,bool))); //Media Player
        connect(this, SIGNAL(thewave_processText(QString,bool)), waveWorker, SLOT(processSpeech(QString,bool))); //Manual Input Text Processing
        connect(this, SIGNAL(thewaveBegin()), waveWorker, SLOT(begin())); //Begin
        connect(this, SIGNAL(thewaveStop()), waveWorker, SLOT(endAndProcess())); //Stop
        connect(this, SIGNAL(thewave_sayLaunchApp(QString)), waveWorker, SLOT(launchAppReply(QString))); //Launch App User Reply
        connect(this, SIGNAL(thewave_sayLaunchApp_disambiguation(QStringList)), waveWorker, SLOT(launchApp_disambiguation(QStringList))); //Lauch App Disambiguation Reply
        connect(this, SIGNAL(currentSettingChanged(bool)), waveWorker, SLOT(currentSettingChanged(bool)));

        connect(waveWorker, SIGNAL(loudnessChanged(qreal)), ui->theWaveFeedback, SLOT(addLevel(qreal)));
        /*connect(w, &speechWorker::outputFrame, [=](QFrame *frame) {
            ui->frame->layout()->addWidget(frame);
        });*/

        t->start();
    }
}

void Menu::thewaveLoudnessChanged(qreal loudness) {
    if (loudness == -1) {
        ui->speechBar->setMaximum(0);
        ui->speechBar->setValue(0);
    } else if (loudness == -2) {
        ui->speechBar->setVisible(false);
    } else {
        ui->speechBar->setMaximum(100);
        ui->speechBar->setValue(loudness * 100);
        ui->speechBar->setVisible(true);
    }
}

void Menu::thewave_outputSpeech(QString speech) {
    QString displaySpeech = speech;
    if (settings.value("thewave/blockOffensiveWords").toBool()) {
        //Sorry... If there's a better way though, that'd be good :)
        displaySpeech.replace("shit", "s•••", Qt::CaseInsensitive);
        displaySpeech.replace("fuck", "f•••", Qt::CaseInsensitive);
        displaySpeech.replace("cunt", "c•••", Qt::CaseInsensitive);
        displaySpeech.replace("bitch", "b••••", Qt::CaseInsensitive);
    }
    ui->thewave_line->setText(displaySpeech);
    ui->theWaveBigSpeechLabel->setText(displaySpeech);
}

void Menu::on_closetheWaveButton_clicked()
{
    if (this->isListening) {
        emit thewaveStop();
    }

    this->istheWaveReady = false;
    this->resetFrames();
    ui->thewave_response->setText(tr("Hit \"Speak\" to start speaking."));
    ui->thewave_line->setText("");

    tPropertyAnimation* anim = new tPropertyAnimation(ui->thewaveFrame, "geometry");
    anim->setStartValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();

    if (waveWorker != NULL) {
        //waveWorker->quit(); //This deletes the worker thread.
        waveWorker->disconnect();
        waveWorker->deleteLater();
        waveWorker = NULL;
    }

    istheWaveOpen = false;
    ui->appsListView->setFocus();
}

void Menu::showCallFrame(bool emergency) {
    Q_UNUSED(emergency)

    ui->thewave_callFrame->setVisible(true);
    ui->thewave_callFrame->setBackupText(tr("Can't place a call from this device."));
}

void Menu::showMessageFrame() {
    ui->thewave_messageframe->setVisible(true);
    ui->thewave_messageframe->setBackupText(tr("Can't send messages from this device."));
}

void Menu::showHelpFrame() {
    ui->thewave_helpFrame->setVisible(true);
    ui->thewave_helpFrame->setBackupText(tr("theWave Help."));
}

void Menu::resetFrames() {
    ui->thewave_callFrame->setVisible(false);
    ui->thewave_messageframe->setVisible(false);
    ui->thewave_helpFrame->setVisible(false);
    ui->wikipediaFrame->setVisible(false);
    ui->thewave_spacerFrame->setVisible(true);
    ui->thewave_launchFrame->setVisible(false);
    ui->thewaveWeatherFrame->setVisible(false);
    ui->thewave_flightFrame->setVisible(false);
    ui->thewaveSettingsFrame->setVisible(false);
    ui->thewaveMathematicsFrame->setVisible(false);
    ui->thewaveMediaFrame->setVisible(false);
}

void Menu::showMediaFrame(QPixmap art, QString title, QString artist, bool isPlaying) {
    ui->thewaveMedia_Art->setPixmap(art.scaled(32, 32));
    ui->thewaveMedia_Title->setText(title);
    ui->thewaveMedia_Artist->setText(artist);
    if (isPlaying) {
        ui->thewaveMedia_Play->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else {
        ui->thewaveMedia_Play->setIcon(QIcon::fromTheme("media-playback-start"));
    }
    ui->thewaveMediaFrame->setVisible(true);

    if (artist == "") {
        ui->thewaveMediaFrame->setBackupText("Now Playing: " + title);
    } else {
        ui->thewaveMediaFrame->setBackupText("Now Playing: " + title + " · " + artist);
    }
}

void Menu::showWikipediaFrame(QString title, QString text) {
    ui->wikipediaTitle->setText(title);
    ui->wikipediaText->setHtml(text);
    ui->wikipediaFrame->setVisible(true);
    ui->thewave_spacerFrame->setVisible(false);
    ui->wikipediaFrame->setBackupText("Wikipedia Content");
}

void Menu::showFlightFrame(QString flight) {
    ui->flightNumber->setText(flight);
    ui->flightImage->setPixmap(QIcon(":/icons/flight/unknown.svg").pixmap(500, 70));
    ui->thewave_flightFrame->setVisible(true);
    ui->wikipediaFrame->setBackupText(flight + " has been cancelled.");
}

void Menu::showSettingFrame(QIcon icon, QString text, bool isOn) {
    ui->thewaveSettingsFrame_icon->setPixmap(icon.pixmap(64, 64));
    ui->thewaveSettingsFrame_Name->setText(text);
    ui->thewaveSettingsFrame_Switch->setChecked(isOn);
    ui->thewaveSettingsFrame->setVisible(true);
    ui->thewaveSettingsFrame->setBackupText(text + " has been switched " + (isOn ? "on" : "off"));
}

void Menu::showMathematicsFrame(QString expression, QString answer) {
    ui->thewaveMathematicsFrame->setVisible(true);
    ui->thewaveMathematics_expression->setText(expression);
    ui->thewaveMathematics_answer->setText(answer);
    ui->thewaveMathematicsFrame->setBackupText(expression + answer);
}

void Menu::on_thewave_line_returnPressed()
{
    while (!istheWaveReady) {
        QApplication::processEvents();
    }
    emit thewave_processText(ui->thewave_line->text());
    thewave_outputSpeech(ui->thewave_line->text());
}

void Menu::on_closetheWaveButton_2_clicked()
{
    QProcess::startDetached("xdg-open https://en.wikipedia.org/wiki/" + ui->wikipediaTitle->text().replace(" ", "_"));
    this->close();
}

void Menu::thewave_launchapp(QString appName) {
    ui->thewave_launchFrame->setVisible(true);
    QList<App> apps;
    for (App app : ((AppsListModel*) ui->appsListView->model())->availableApps()) {
        if (app.name().remove(" ").contains(appName.remove(" "), Qt::CaseInsensitive)) {
            apps.append(app);
        }
    }

    if (apps.count() == 0) {
        ui->thewave_launch_appIcon->setVisible(false);
        ui->thewave_launch_appName->setVisible(false);
        ui->thewave_launch_error->setVisible(true);
        ui->thewave_launch_launchapp->setVisible(false);
        ui->thewave_spacerFrame->setVisible(true);
    } else if (apps.count() == 1) {
        ui->thewave_launch_appIcon->setVisible(true);
        ui->thewave_launch_appName->setVisible(true);
        ui->thewave_launch_error->setVisible(false);
        ui->thewave_launchOneAppFrame->setVisible(true);
        ui->thewave_launch_disambiguation->setVisible(false);
        ui->thewave_launch_launchapp->setVisible(true);
        ui->thewave_spacerFrame->setVisible(true);

        ui->thewave_launch_appName->setText(apps.first().name());
        ui->thewave_launch_appIcon->setPixmap(apps.first().icon().pixmap(64));
        ui->thewave_launch_launchapp->setProperty("appcommand", apps.first().command());


        emit thewave_sayLaunchApp(apps.first().name());
    } else {
        ui->thewave_launch_error->setVisible(false);
        ui->thewave_launchOneAppFrame->setVisible(false);
        ui->thewave_launch_disambiguation->setVisible(true);
        ui->thewave_spacerFrame->setVisible(false);

        ui->thewave_launch_disambiguation->clear();
        QStringList appNames;
        for (App app : apps) {
            QListWidgetItem* item = new QListWidgetItem();
            item->setText(app.name());
            item->setIcon(app.icon());
            item->setData(Qt::UserRole, app.command());
            ui->thewave_launch_disambiguation->addItem(item);
            appNames.append(app.name());
        }

        emit (thewave_sayLaunchApp_disambiguation(appNames));
    }
}

void Menu::on_thewave_launch_launchapp_clicked()
{
    QProcess::startDetached(ui->thewave_launch_launchapp->property("appcommand").toString().remove("%u"));
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

void Menu::on_listentheWave_clicked()
{
    if (this->isListening) {
        emit thewaveStop();
    } else {
        emit thewaveBegin();
    }
}

void Menu::on_thewave_launch_disambiguation_itemClicked(QListWidgetItem *item)
{
    QProcess::startDetached(item->data(Qt::UserRole).toString().remove("%u"));
    this->close();
}

void Menu::on_thewaveSettingsFrame_Switch_toggled(bool checked)
{
    emit currentSettingChanged(checked);
}

void Menu::on_thewaveMedia_Next_clicked()
{
    MainWin->nextSong();
}

void Menu::on_thewaveMedia_Play_clicked()
{
    MainWin->playPause();
}

void Menu::on_thewaveMedia_Back_clicked()
{
    MainWin->previousSong();
}

void Menu::mousePressEvent(QMouseEvent *event) {

}

void Menu::mouseMoveEvent(QMouseEvent *event) {

}

void Menu::mouseReleaseEvent(QMouseEvent *event) {

}

theWaveFrame::theWaveFrame(QWidget *parent) : QFrame(parent)
{

}

void theWaveFrame::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        this->startPosition = event->pos();
    }
}

void theWaveFrame::mouseMoveEvent(QMouseEvent *event)
 {
    if (event->buttons() & Qt::LeftButton && ((event->pos() - this->startPosition).manhattanLength() >= QApplication::startDragDistance())) {
        QPixmap pixmap = this->grab();
        QByteArray pngArray;
        QBuffer buffer(&pngArray);
        buffer.open(QIODevice::WriteOnly);
        pixmap.save(&buffer, "PNG");
        buffer.close();

        QDrag *drag = new QDrag(this);
        QMimeData *mime = new QMimeData();
        mime->setData("image/png", pngArray);
        /*if (this->backupText() != "") {
            //mime->setText(this->backupText());
            mime->setText(this->backupText());
        }*/
        drag->setMimeData(mime);
        drag->setPixmap(pixmap);
        ((Menu*) this->window())->collapse();
        drag->exec();
        ((Menu*) this->window())->uncollapse();
    }
}

void theWaveFrame::setBackupText(QString text) {
    this->bText = text;
}

QString theWaveFrame::backupText() {
    return this->bText;
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

void Menu::on_appsListView_clicked(const QModelIndex &index)
{
    if (((AppsListModel*) ui->appsListView->model())->launchApp(index)) {
        this->close();
    }
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
