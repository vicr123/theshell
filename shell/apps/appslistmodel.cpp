#include "appslistmodel.h"

extern float getDPIScaling();
extern NativeEventFilter* NativeFilter;
extern MainWindow* MainWin;
extern QSettings::Format desktopFileFormat;

AppsListModel::AppsListModel(BTHandsfree* bt, QObject *parent) : QAbstractListModel(parent) {
    this->bt = bt;
    loadData();

    connect(NativeFilter, SIGNAL(DoRetranslation()), this, SLOT(loadData()));
}

AppsListModel::~AppsListModel() {

}

int AppsListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return appsShown.count();
}

QVariant AppsListModel::data(const QModelIndex &index, int role) const {
    if (appsShown.count() > index.row()) {
        if (role == Qt::DisplayRole) {
            return appsShown.at(index.row()).name();
        } else if (role == Qt::DecorationRole) {
            return appsShown.at(index.row()).icon().pixmap(32 * getDPIScaling(), 32 * getDPIScaling());
        } else if (role == Qt::UserRole) { //Description
            if (appsShown.at(index.row()).description() == "") {
                return tr("Application");
            } else {
                return appsShown.at(index.row()).description();
            }
        } else if (role == Qt::UserRole + 1) { //Pinned
            return appsShown.at(index.row()).isPinned();
        } else if (role == Qt::UserRole + 2) { //Desktop Entry
            return appsShown.at(index.row()).desktopEntry();
        } else if (role == Qt::UserRole + 3) { //App
            return QVariant::fromValue(appsShown.at(index.row()));
        }
    }
    return QVariant();
}

void AppsListModel::updateData() {
    emit dataChanged(index(0), index(rowCount()));
}

void AppsListModel::search(QString query) {
    currentQuery = query;
    appsShown.clear();
    if (query == "") {
        appsShown.append(apps);
        updateData();
    } else {
        if (query.toLower().startsWith("call")) {
            QString number = query.mid(5);
            if (number != "" && this->bt != NULL) {
                QStringList devices = bt->getDevices();
                for (int i = 0; i < devices.count(); i++) {
                    App app;
                    app.setName(number);
                    app.setCommand("call:" + QString::number(i) + ":" + number);
                    app.setDescription(tr("Place a call over ") + devices.at(i));
                    app.setIcon(QIcon::fromTheme("call-start"));
                    appsShown.append(app);
                }
            }
        }

        int i = 0;
        for (App app : apps) {
            if (i >= pinnedAppsCount || pinnedAppsCount == 0) {
                if (app.name().contains(query, Qt::CaseInsensitive) || app.description().contains(query, Qt::CaseInsensitive)) {
                    appsShown.append(app);
                }
            }
            i++;
        }

        /*if (QString("shutdown").contains(arg1, Qt::CaseInsensitive) || QString("power off").contains(arg1, Qt::CaseInsensitive)) {
            QListWidgetItem *i = new QListWidgetItem();
            i->setText(tr("Power Off"));
            i->setIcon(QIcon::fromTheme("system-shutdown"));
            i->setData(Qt::UserRole, "power:off");
            ui->listWidget->addItem(i);
        } else if (QString("restart").contains(arg1, Qt::CaseInsensitive) || QString("reboot").contains(arg1, Qt::CaseInsensitive)) {
            QListWidgetItem *i = new QListWidgetItem();
            i->setText(tr("Reboot"));
            i->setIcon(QIcon::fromTheme("system-reboot"));
            i->setData(Qt::UserRole, "power:reboot");
            ui->listWidget->addItem(i);
        } else if (QString("logout").contains(arg1, Qt::CaseInsensitive) || QString("logoff").contains(arg1, Qt::CaseInsensitive)) {
            QListWidgetItem *i = new QListWidgetItem();
            i->setText(tr("Log Out"));
            i->setIcon(QIcon::fromTheme("system-log-out"));
            i->setData(Qt::UserRole, "power:logout");
            ui->listWidget->addItem(i);
        }*/

        QString pathEnv = QProcessEnvironment::systemEnvironment().value("PATH");
        for (QString env : pathEnv.split(":")) {
            if (QFile(env.append("/" + query.split(" ")[0])).exists()) {
                App app;
                app.setName(query);
                app.setCommand(query);
                app.setDescription(tr("Run Command"));
                app.setIcon(QIcon::fromTheme("system-run"));
                appsShown.append(app);
                break;
            }
        }

        QUrl uri = QUrl::fromUserInput(query);
        if (uri.scheme() == "http" || uri.scheme() == "https") {
            App app;
            app.setName(uri.toDisplayString());
            app.setDescription(tr("Open webpage"));
            app.setCommand("xdg-open \"" + uri.toString() + "\"");
            app.setIcon(QIcon::fromTheme("text-html"));
            appsShown.append(app);
        } else if (uri.scheme() == "file") {
            if (QDir(uri.path() + "/").exists()) {
                App app;
                app.setName(uri.path());
                app.setDescription(tr("Open Folder"));
                app.setCommand("xdg-open \"" + uri.toString() + "\"");
                app.setIcon(QIcon::fromTheme("system-file-manager"));
                appsShown.append(app);
            } else if (QFile(uri.path()).exists()) {
                App app;
                app.setName(uri.path());
                app.setDescription(tr("Open File"));
                app.setCommand("xdg-open \"" + uri.toString() + "\"");
                QFile f(uri.toString());
                QFileInfo info(f);
                QMimeType mime = (new QMimeDatabase())->mimeTypeForFile(info);
                app.setIcon(QIcon::fromTheme(mime.iconName(), QIcon::fromTheme("application-octet-stream")));
                appsShown.append(app);
            }
        }

        updateData();
    }
}

void AppsListModel::loadData() {
    if (loadDataFuture.isRunning()) {
        queueLoadData = true;
    } else {
        loadDataFuture = QtConcurrent::run([=]() -> dataLoad {
            QList<App> apps;
            int pinnedAppsCount;
            QStringList appList, pinnedAppsList;

            settings.beginGroup("gateway");
            int count = settings.beginReadArray("pinnedItems");
            for (int i = 0; i < count; i++) {
                settings.setArrayIndex(i);
                //appList.append(settings.value("desktopEntry").toString());
                pinnedAppsList.append(settings.value("desktopEntry").toString());
            }
            settings.endArray();
            settings.endGroup();
            pinnedAppsCount = pinnedAppsList.count();

            QDir appFolder("/usr/share/applications/");
            QDirIterator* iterator = new QDirIterator(appFolder, QDirIterator::Subdirectories);

            while (iterator->hasNext()) {
                appList.append(iterator->next());
            }

            delete iterator;

            App settingsApp;
            settingsApp.setCommand("sc:settings");
            settingsApp.setIcon(QIcon::fromTheme("configure"));
            settingsApp.setName(tr("System Settings"));
            settingsApp.setDescription(tr("System Configuration"));
            apps.append(settingsApp);

            appFolder = QDir(QDir::homePath() + "/.local/share/applications");
            if (appFolder.exists()) {
                iterator = new QDirIterator(appFolder, QDirIterator::Subdirectories);
                while (iterator->hasNext()) {
                    appList.append(iterator->next());
                }
                delete iterator;
            }

            for (QString appFile : appList) {
                App app = readAppFile(appFile, pinnedAppsList);
                if (!app.invalid()) {
                    apps.prepend(app);
                }
            }

            std::sort(apps.begin(), apps.end());

            for (int i = pinnedAppsList.count() - 1; i >= 0; i--) {
                App app = readAppFile(pinnedAppsList.at(i), pinnedAppsList);
                if (!app.invalid()) {
                    apps.prepend(app);
                }
            }

            dataLoad r;
            r.apps = apps;
            r.pinnedAppsCount = pinnedAppsCount;
            return r;
        });

        QFutureWatcher<dataLoad>* watcher = new QFutureWatcher<dataLoad>();
        connect(watcher, &QFutureWatcher<dataLoad>::finished, [=] {
            watcher->deleteLater();
            dataLoad r = loadDataFuture.result();
            this->apps = r.apps;
            this->pinnedAppsCount = r.pinnedAppsCount;

            if (queueLoadData) {
                queueLoadData = false;
                loadData();
            } else {
                search(currentQuery);
            }
        });
        watcher->setFuture(loadDataFuture);
    }
}

QList<App> AppsListModel::availableApps() {
    return apps;
}

App AppsListModel::readAppFile(QString appFile, QStringList pinnedAppsList) {
    /*QFile file(appFile);
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

        QString lang = QLocale().name().split('_').first();

        for (QString desktopPart : desktopLines) {
            App app;
            app.setDesktopEntry(appFile);

            if (pinnedAppsList.contains(appFile)) {
                app.setPinned(true);
            }

            bool isApplication = false;
            bool display = true;
            QString name, localName = "";
            for (QString line : desktopPart.split("\n")) {
                if (line.startsWith("genericname=", Qt::CaseInsensitive)) {
                    app.setDescription(line.split("=")[1]);
                } else if (line.startsWith("name=", Qt::CaseInsensitive)) {
                    name = line.split("=")[1];
                } else if (line.startsWith("name[" + lang + "]=", Qt::CaseInsensitive)) {
                    localName = line.split("=")[1];
                } else if (line.startsWith("icon=", Qt::CaseInsensitive)) {
                    QString iconname = line.split("=")[1];
                    QIcon icon;
                    if (QFile(iconname).exists()) {
                        icon = QIcon(iconname);
                    } else {
                        icon = QIcon::fromTheme(iconname, QIcon::fromTheme("application-x-executable"));
                    }
                    app.setIcon(icon);
                } else if (line.startsWith("exec=", Qt::CaseInsensitive)) {
                    QStringList command = line.split("=");
                    command.removeFirst();

                    QString commandLine = command.join("=");
                    commandLine.remove("%u");
                    commandLine.remove("%U");
                    commandLine.remove("%f");
                    commandLine.remove("%F");
                    commandLine.remove("%k");
                    commandLine.remove("%i");
                    app.setCommand(commandLine);
                } else if (line.startsWith("description=", Qt::CaseInsensitive)) {
                    app.setDescription(line.split("=")[1]);
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

            if (localName != "") {
                app.setName(localName);
            } else {
                app.setName(name);
            }
            app.setCommand(app.command().replace("%c", "\"" + app.name() + "\""));

            if (isApplication && display) {
                return app;
            }
        }
    }
    */
    QSettings desc(appFile, desktopFileFormat);
    App app;
    QString lang = QLocale().name().split('_').first();
    desc.beginGroup("Desktop Entry");

    if (desc.value("Type", "").toString() != "Application") {
        return App::invalidApp();
    }
    if (desc.value("NoDisplay", false).toBool() == true) {
        return App::invalidApp();
    }
    if (!desc.value("OnlyShowIn", "theshell;").toString().contains("theshell;")) {
        return App::invalidApp();
    }
    if (desc.value("NotShowIn", "").toString().contains("theshell;")) {
        return App::invalidApp();
    }

    if (desc.contains("Name[" + lang + "]")) {
        app.setName(desc.value("Name[" + lang + "]").toString());
    } else {
        app.setName(desc.value("Name").toString());
    }

    QString commandLine = desc.value("Exec").toString();
    commandLine.remove("%u");
    commandLine.remove("%U");
    commandLine.remove("%f");
    commandLine.remove("%F");
    commandLine.remove("%k");
    commandLine.remove("%i");
    commandLine.replace("%c", "\"" + app.name() + "\"");
    app.setCommand(commandLine);

    if (desc.contains("GenericName")) {
        app.setDescription(desc.value("GenericName").toString());
    }

    if (desc.contains("Icon")) {
        QString iconname = desc.value("Icon").toString();
        QIcon icon;
        if (QFile(iconname).exists()) {
            icon = QIcon(iconname);
        } else {
            icon = QIcon::fromTheme(iconname, QIcon::fromTheme("application-x-executable"));
        }
        app.setIcon(icon);
    }

    if (desc.contains("Actions")) {
        qDebug() << desc.value("Actions").toString();
        QStringList availableActions = desc.value("Actions").toString().split(";", QString::SkipEmptyParts);
        desc.endGroup();

        for (QString action : availableActions) {
            desc.beginGroup("Desktop Action " + action);

            App act;
            act.setName(desc.value("Name").toString());
            act.setCommand(desc.value("Exec").toString());
            app.addAction(act);

            desc.endGroup();
        }
    }
    desc.endGroup();

    if (pinnedAppsList.contains(appFile)) {
        app.setPinned(true);
    }

    app.setDesktopEntry(appFile);
    return app;
}

AppsDelegate::AppsDelegate(QWidget *parent) : QStyledItemDelegate(parent) {

}

void AppsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->setFont(option.font);

    QRect iconRect;
    if (((QListView*) option.widget)->viewMode() == QListView::IconMode) {
        iconRect.setLeft(option.rect.left() + 32 * getDPIScaling());
        iconRect.setTop(option.rect.top() + 6 * getDPIScaling());
        iconRect.setHeight(32 * getDPIScaling());
        iconRect.setWidth(32 * getDPIScaling());

        QRect textRect;
        textRect.setLeft(option.rect.left() + 6 * getDPIScaling());
        textRect.setTop(iconRect.bottom() + 6 * getDPIScaling());
        textRect.setBottom(option.rect.bottom());
        textRect.setRight(option.rect.right());

        if (option.state & QStyle::State_Selected) {
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::Highlight));
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::HighlightedText));
            painter->drawText(textRect, index.data().toString());
        } else if (option.state & QStyle::State_MouseOver) {
            QColor col = option.palette.color(QPalette::Highlight);
            col.setAlpha(127);
            painter->setBrush(col);
            painter->setPen(Qt::transparent);
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
        } else {
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
        }
    } else {
        iconRect.setLeft(option.rect.left() + 6 * getDPIScaling());
        iconRect.setTop(option.rect.top() + 6 * getDPIScaling());
        iconRect.setBottom(iconRect.top() + 32 * getDPIScaling());
        iconRect.setRight(iconRect.left() + 32 * getDPIScaling());

        QRect textRect;
        textRect.setLeft(iconRect.right() + 6 * getDPIScaling());
        textRect.setTop(option.rect.top() + 6 * getDPIScaling());
        textRect.setBottom(option.rect.top() + option.fontMetrics.height() + 6 * getDPIScaling());
        textRect.setRight(option.rect.right());

        QRect descRect;
        descRect.setLeft(iconRect.right() + 6 * getDPIScaling());
        descRect.setTop(option.rect.top() + option.fontMetrics.height() + 8 * getDPIScaling());
        descRect.setBottom(option.rect.top() + option.fontMetrics.height() * 2 + 6 * getDPIScaling());
        descRect.setRight(option.rect.right());

        if (option.state & QStyle::State_Selected) {
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::Highlight));
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::HighlightedText));
            painter->drawText(textRect, index.data().toString());
            painter->drawText(descRect, index.data(Qt::UserRole).toString());
        } else if (option.state & QStyle::State_MouseOver) {
            QColor col = option.palette.color(QPalette::Highlight);
            col.setAlpha(127);
            painter->setBrush(col);
            painter->setPen(Qt::transparent);
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(descRect, index.data(Qt::UserRole).toString());
        } else {
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, index.data().toString());
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(descRect, index.data(Qt::UserRole).toString());
        }
    }
    painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QPixmap>());

    App a = index.data(Qt::UserRole + 3).value<App>();
    if (a.actions().count() > 0) { //Actions included
        QRect actionsRect;
        actionsRect.setWidth(16 * getDPIScaling());
        actionsRect.setHeight(16 * getDPIScaling());
        actionsRect.moveRight(option.rect.right() - 9 * getDPIScaling());
        actionsRect.moveTop(option.rect.top() + option.rect.height() / 2 - 8 * getDPIScaling());

        painter->drawPixmap(actionsRect, QIcon::fromTheme("arrow-right").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
    }

    int pinned = ((AppsListModel*) index.model())->pinnedAppsCount;
    if (index.row() == pinned - 1 && ((AppsListModel*) index.model())->currentQuery == "") {
        painter->setPen(option.palette.color(QPalette::WindowText));
        painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
    }
}

QSize AppsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    //int height;

    if (((QListView*) option.widget)->viewMode() == QListView::IconMode) {
        return QSize(128 * getDPIScaling(), 128 * getDPIScaling());
    } else {
        int fontHeight = option.fontMetrics.height() * 2 + 14 * getDPIScaling();
        int iconHeight = 46 * getDPIScaling();

        return QSize(option.fontMetrics.width(index.data().toString()), qMax(fontHeight, iconHeight));
    }
}

bool AppsListModel::launchApp(QModelIndex index) {

    /*
    } else if (item->data(Qt::UserRole).toString().startsWith("power:")) {
        QString operation = item->data(Qt::UserRole).toString().split(":").at(1);
        if (operation == "off") {
            EndSession(EndSessionWait::powerOff);
        } else if (operation == "reboot") {
            EndSession(EndSessionWait::reboot);
        } else if (operation ==  "logout") {
            EndSession(EndSessionWait::logout);
        }
    } else if (item->data(Qt::UserRole).toString().startsWith("media:")) {
        QString operation = item->data(Qt::UserRole).toString().split(":").at(1);
        if (operation == "play") {
            MainWin->play();
        } else if (operation == "pause") {
            MainWin->pause();
        } else if (operation ==  "next") {
            MainWin->nextSong();
        } else if (operation ==  "previous") {
            MainWin->previousSong();
        }
        MainWin->doUpdate();
        QThread::msleep(50);
        on_lineEdit_textEdited(ui->lineEdit->text());
    } else {*/
        QString command = appsShown.at(index.row()).command().remove("%u");
        if (command.startsWith("call:")) {
            QString callCommand = command.mid(5);
            QStringList parts = callCommand.split(":");
            int deviceIndex = parts.at(0).toInt();
            QString number = parts.at(1);
            bt->placeCall(deviceIndex, number);
            return true;
        } else if (command == "sc:settings") {
            MainWin->getInfoPane()->show(InfoPaneDropdown::Settings);
            return true;
        } else {
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
            return true;
        }
        //QProcess::startDetached(item->data(Qt::UserRole).toString().remove("%u"));
    //}
}
