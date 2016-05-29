#include "menu.h"
#include "ui_menu.h"

extern void EndSession(EndSessionWait::shutdownType type);
extern MainWindow* MainWin;

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
    ui->timerIcon->setPixmap(QIcon::fromTheme("player-time").pixmap(16));
    ui->userIcon->setPixmap(QIcon::fromTheme("system-users").pixmap(16));

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
    QString fullname = parseName.split(",").at(0).split(":").last();
    if (fullname == "") {
        ui->label_2->setText("Hey, " + name + "!");
        //ui->label_4->setText(name + ", what do you want to do now?");
    } else {
        ui->label_2->setText("Hey, " + fullname + "!");
        //ui->label_4->setText(fullname + ", what do you want to do now?");
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
                    } else if (line.startsWith("onlyshowin=", Qt::CaseInsensitive)) {
                        if (!line.split("=")[1].contains("theshell;")) {
                            display = false;
                        }
                    } else if (line.startsWith("notshowin=", Qt::CaseInsensitive)) {
                        if (line.split("=")[1].contains("theshell;")) {
                            display = false;
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
            i->setText(app->description() + " | " + app->name());
        }
        i->setIcon(app->icon());
        i->setData(Qt::UserRole, app->command());
        appsShown->append(app);
        ui->listWidget->addItem(i);
    }

    QPropertyAnimation* animation = new QPropertyAnimation(this, "geometry");
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(this->x() + this->width(), this->y(), this->width(), this->height()));
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start();
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    /*QTimer *t = new QTimer(this);
    t->setInterval(1);
    connect(t, SIGNAL(timeout()), this, SLOT(checkForclose()));
    t->start();*/
}

void Menu::changeEvent(QEvent *event) {
    QDialog::changeEvent(event);
    if (event->type() == QEvent::ActivationChange) {
        if (!this->isActiveWindow()) {
            this->close();
        }
    }
}

void Menu::close() {
    QPropertyAnimation* animation = new QPropertyAnimation(this, "geometry");
    animation->setStartValue(this->geometry());
    animation->setEndValue(QRect(this->x() - this->width(), this->y(), this->width(), this->height()));
    animation->setDuration(500);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start();
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));
    connect(animation, &QPropertyAnimation::finished, [=]() {
        emit menuClosing();
        QDialog::close();
    });

    doCheckForClose = false;
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
        ui->shutdownText->setText("Before you power off your PC, you may want to check this.");
        ui->shutdownWarnings->setVisible(true);
    } else {
        ui->shutdownText->setText("You're about to power off your PC. Are you sure?");
        ui->shutdownWarnings->setVisible(false);
    }

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

void Menu::on_listWidget_itemClicked(QListWidgetItem *item)
{
    /*for (App* app : *appsShown) {
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
    }*/

    QProcess::startDetached(item->data(Qt::UserRole).toString().remove("%u"));
    //emit appOpening(app->name(), app->icon());
    this->close();

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
            i->setData(Qt::UserRole, app->command());
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
                i->setData(Qt::UserRole, app->command());
                appsShown->append(app);
                ui->listWidget->addItem(i);

            }
        }

        if (QString("shutdown").contains(arg1, Qt::CaseInsensitive) || QString("power off").contains(arg1, Qt::CaseInsensitive)) {
            QListWidgetItem *i = new QListWidgetItem();
            i->setText("Power Off");
            i->setIcon(QIcon::fromTheme("system-shutdown"));
            i->setData(Qt::UserRole, "POWEROFF");
            ui->listWidget->addItem(i);
        } else if (QString("restart").contains(arg1, Qt::CaseInsensitive) || QString("reboot").contains(arg1, Qt::CaseInsensitive)) {
            QListWidgetItem *i = new QListWidgetItem();
            i->setText("Reboot");
            i->setIcon(QIcon::fromTheme("system-reboot"));
            i->setData(Qt::UserRole, "REBOOT");
            ui->listWidget->addItem(i);
        }

        QString pathEnv = QProcessEnvironment::systemEnvironment().value("PATH");
        for (QString env : pathEnv.split(":")) {
            if (QFile(env.append("/" + arg1.split(" ")[0])).exists()) {
                App* app = new App();
                app->setName(arg1);
                app->setCommand(arg1);
                app->setIcon(QIcon::fromTheme("system-run"));
                appsShown->append(app);

                QListWidgetItem *i = new QListWidgetItem();
                i->setText(app->name());
                i->setIcon(app->icon());
                i->setData(Qt::UserRole, app->command());
                ui->listWidget->addItem(i);
                break;
            }
        }
    }

    QUrl uri = QUrl::fromUserInput(arg1);
    if (uri.scheme() == "http" || uri.scheme() == "https") {
        App* app = new App();
        app->setName("Go to " + uri.toDisplayString());
        app->setCommand("xdg-open \"" + uri.toString() + "\"");
        app->setIcon(QIcon::fromTheme("text-html"));
        appsShown->append(app);

        QListWidgetItem *i = new QListWidgetItem();
        i->setText(app->name());
        i->setIcon(app->icon());
        i->setData(Qt::UserRole, app->command());
        ui->listWidget->addItem(i);
    } else if (uri.scheme() == "file") {
        if (QDir(uri.path() + "/").exists()) {
            App* app = new App();
            app->setName("Open " + uri.path());
            app->setCommand("xdg-open \"" + uri.toString() + "\"");
            app->setIcon(QIcon::fromTheme("system-file-manager"));
            appsShown->append(app);

            QListWidgetItem *i = new QListWidgetItem();
            i->setText(app->name());
            i->setIcon(app->icon());
            i->setData(Qt::UserRole, app->command());
            ui->listWidget->addItem(i);
        } else if (QFile(uri.path()).exists()) {
            App* app = new App();
            app->setName("Open " + uri.path());
            app->setCommand("xdg-open \"" + uri.toString() + "\"");
            QFile f(uri.toString());
            QFileInfo info(f);
            QMimeType mime = (new QMimeDatabase())->mimeTypeForFile(info);
            app->setIcon(QIcon::fromTheme(mime.iconName(), QIcon::fromTheme("application-octet-stream")));
            appsShown->append(app);

            QListWidgetItem *i = new QListWidgetItem();
            i->setText(app->name());
            i->setIcon(app->icon());
            i->setData(Qt::UserRole, app->command());
            ui->listWidget->addItem(i);
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
    QList<QVariant> arguments;
    arguments.append(true);

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Suspend");
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
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
    QList<QVariant> arguments;
    arguments.append(true);

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "Hibernate");
    message.setArguments(arguments);
    QDBusConnection::systemBus().send(message);
}

void Menu::on_commandLinkButton_8_clicked()
{
    this->close();
    QProcess::startDetached("xset dpms force off");
}