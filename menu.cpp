#include "menu.h"
#include "ui_menu.h"

Menu::Menu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);

    ui->offFrame->setParent(this);
    this->layout()->removeWidget(ui->offFrame);
    ui->offFrame->setGeometry(10, -this->height(), this->width() - 20, this->height() - 20);
    ui->commandLinkButton->setStyleSheet("background-color: #A00;");
    ui->commandLinkButton_2->setStyleSheet("background-color: #A00;");

    this->setMouseTracking(true);

    QTimer *t = new QTimer(this);
    t->setInterval(500);
    connect(t, SIGNAL(timeout()), this, SLOT(checkForclose()));
    t->start();

    QString name = qgetenv("USER");
    if (name.isEmpty()) {
        name = qgetenv("USERNAME");
    }

    QProcess* fullNameProc = new QProcess(this);
    fullNameProc->start("getent passwd " + name);
    fullNameProc->waitForFinished();
    QString parseName(fullNameProc->readAll());
    delete fullNameProc;
    QString fullname = parseName.split(",").at(0).split(":").last();
    if (fullname == "") {
        ui->label_2->setText("Hey, " + name + "!");
        ui->label_4->setText(name + ", what do you want to do after we exit?");
    } else {
        ui->label_2->setText("Hey, " + fullname + "!");
        ui->label_4->setText(fullname + ", what do you want to do after we exit?");
    }


    apps = new QList<App*>();
    appsShown = new QList<App*>();

    ui->listWidget->installEventFilter(this);

    this->installEventFilter(this);

    if (QFile("/usr/bin/install_theos").exists()) {
        ui->InstallLayout->setVisible(true);
    } else {
        ui->InstallLayout->setVisible(false);
    }
}

Menu::~Menu()
{
    delete ui;
}

void Menu::show() {
    QDialog::show();
    doCheckForClose = true;

    QDir appFolder("/usr/share/applications/");
    QDirIterator iterator(appFolder, QDirIterator::Subdirectories);

    while (iterator.hasNext()) {
        QString appFile = iterator.next();
        QFile file(appFile);
        if (file.exists() & QFileInfo(file).suffix().contains("desktop")) {
            file.open(QFile::ReadOnly);
            QString appinfo(file.readAll());

            QStringList desktopLines;
            QString currentDesktopLine;
            for (QString desktopLine : appinfo.split("\n")) {
                if (desktopLine.startsWith("[") && currentDesktopLine != "") {
                    desktopLines.append(currentDesktopLine);
                    currentDesktopLine = "";
                }
                currentDesktopLine.append(desktopLine + "\n");
            }
            desktopLines.append(currentDesktopLine);

            for (QString desktopPart : desktopLines) {
                App *app = new App();
                bool isApplication = false;
                bool display = true;
                for (QString line : desktopPart.split("\n")) {
                    if (line.startsWith("genericname=", Qt::CaseInsensitive)) {
                        app->setDescription(line.split("=")[1]);
                    } else if (line.startsWith("name=", Qt::CaseInsensitive)) {
                        app->setName(line.split("=")[1]);
                    } else if (line.startsWith("icon=", Qt::CaseInsensitive)) {
                        QString iconname = line.split("=")[1];
                        QIcon icon;
                        if (QFile(iconname).exists()) {
                            icon = QIcon(iconname);
                        } else {
                            icon = QIcon::fromTheme(iconname, QIcon::fromTheme("application-x-executable"));
                        }
                        app->setIcon(icon);
                    } else if (line.startsWith("exec=", Qt::CaseInsensitive)) {
                        app->setCommand(line.split("=")[1].remove(QRegExp("%.")));
                    } else if (line.startsWith("description=", Qt::CaseInsensitive)) {
                        app->setDescription(line.split("=")[1]);
                    } else if (line.startsWith("type=", Qt::CaseInsensitive)) {
                        if (line.split("=")[1] == "Application") {
                            isApplication = true;
                        }
                    } else if (line.startsWith("nodisplay=", Qt::CaseInsensitive)) {
                        if (line.split("=")[1] == "true") {
                            display = false;
                            break;
                        }
                    }
                }
                if (isApplication && display) {
                    apps->append(app);
                }
            }
        }
    }
    //hotkeyManager->registerHotkey("Power");

    for (App *app : *apps) {
        QListWidgetItem *i = new QListWidgetItem();
        if (app->description() == "") {
            i->setText(app->name());
        } else {
            i->setText(app->name() + " | " + app->description());
        }
        i->setIcon(app->icon());
        appsShown->append(app);
        ui->listWidget->addItem(i);
    }
}

void Menu::close() {
    doCheckForClose = false;
    emit menuClosing();
    QDialog::close();
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
    QPropertyAnimation* anim = new QPropertyAnimation(ui->offFrame, "geometry");
    anim->setStartValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setDuration(500);

    anim->setEasingCurve(QEasingCurve::OutCubic);

    anim->start();
}

void Menu::on_pushButton_2_clicked()
{
    QPropertyAnimation* anim = new QPropertyAnimation(ui->offFrame, "geometry");
    anim->setStartValue(QRect(10, 10, this->width() - 20, this->height() - 20));
    anim->setEndValue(QRect(10, this->height(), this->width() - 20, this->height() - 20));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    anim->start();
}

void Menu::on_commandLinkButton_clicked()
{
    EndSessionWait* w = new EndSessionWait(EndSessionWait::powerOff);
    w->showFullScreen();
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void Menu::on_commandLinkButton_2_clicked()
{
    EndSessionWait* w = new EndSessionWait(EndSessionWait::reboot);
    w->showFullScreen();
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void Menu::on_commandLinkButton_3_clicked()
{
    EndSessionWait* w = new EndSessionWait(EndSessionWait::logout);
    w->showFullScreen();
    QApplication::setOverrideCursor(Qt::BlankCursor);
}

void Menu::on_listWidget_itemClicked(QListWidgetItem *item)
{
    for (App* app : *appsShown) {
        bool correctApp = false;
        if (item->text().contains("|")) {
            if (app->name() == item->text().split("|")[1].remove(0, 1)) {
                correctApp = true;
            }
        } else if (app->name() == item->text()) {
            correctApp = true;
        }
        if (correctApp) {
            QProcess::startDetached(app->command().remove("%u"));
            emit appOpening(app->name(), app->icon());
            this->close();
            break;
        }
    }
    //App *app = appsShown->at(ui->listWidget->selectionModel()->selectedIndexes().at(0).row());
}
void Menu::on_lineEdit_textChanged(const QString &arg1)
{
}

void Menu::on_lineEdit_textEdited(const QString &arg1)
{
    ui->listWidget->clear();
    appsShown->clear();
    if (arg1 == "") {
        for (App *app : *apps) {
            QListWidgetItem *i = new QListWidgetItem();
            if (app->description() == "") {
                i->setText(app->name());
            } else {
                i->setText(app->description() + " | " + app->name());
            }
            i->setIcon(app->icon());
            appsShown->append(app);
            ui->listWidget->addItem(i);
        }
    } else {
        for (App *app : *apps) {
            if (app->name().contains(arg1, Qt::CaseInsensitive) || app->description().contains(arg1, Qt::CaseInsensitive)) {
                QListWidgetItem *i = new QListWidgetItem();
                if (app->description() == "") {
                    i->setText(app->name());
                } else {
                    i->setText(app->description() + " | " + app->name());
                }
                i->setIcon(app->icon());
                appsShown->append(app);
                ui->listWidget->addItem(i);

            }
        }

        QString pathEnv = QProcessEnvironment::systemEnvironment().value("PATH");
        for (QString env : pathEnv.split(":")) {
            if (QFile(env.append("/" + arg1.split(" ")[0])).exists()) {
                App* app = new App();
                app->setName(arg1);
                app->setCommand(arg1);
                app->setIcon(QIcon::fromTheme("application-x-executable"));
                appsShown->append(app);

                QListWidgetItem *i = new QListWidgetItem();
                if (app->description() == "") {
                    i->setText(app->name());
                } else {
                    i->setText(app->description() + "(" + app->name() + ")");
                }
                i->setIcon(app->icon());
                ui->listWidget->addItem(i);
                break;
            }
        }
    }

}

bool Menu::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = (QKeyEvent*) event;
        ui->lineEdit->setText(ui->lineEdit->text().append(e->text()));
        ui->lineEdit->setFocus();
        on_lineEdit_textEdited(ui->lineEdit->text());
        //ui->lineEdit->keyPressEvent(e);
        e->ignore();
        return true;
    } else {
        return QDialog::eventFilter(object, event);
    }
}

void Menu::on_lineEdit_returnPressed()
{
    if (ui->listWidget->count() > 0) {
        ui->listWidget->selectedItems().append(ui->listWidget->item(0));
        on_listWidget_itemClicked(ui->listWidget->item(0));
    }
}

void Menu::on_pushButton_3_clicked()
{
    QProcess::startDetached("install_theos");
    emit appOpening("theOS Installer", QIcon::fromTheme("install_theos"));
    this->close();
}

void Menu::on_commandLinkButton_5_clicked()
{
    QProcess::startDetached("systemctl suspend");
    this->close();
}
