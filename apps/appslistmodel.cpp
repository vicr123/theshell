#include "appslistmodel.h"

extern float getDPIScaling();
extern NativeEventFilter* NativeFilter;
extern MainWindow* MainWin;

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

        bool showtheWaveOption = true;
        /*if (settings.value("thewave/enabled", true).toBool()) {
            if (arg1.toLower() == "emergency call") {
                QListWidgetItem *callItem = new QListWidgetItem();
                callItem->setText(tr("Place a call"));
                callItem->setIcon(QIcon::fromTheme("call-start"));
                callItem->setData(Qt::UserRole, "thewave:emergency call");
                ui->listWidget->addItem(callItem);

                QListWidgetItem *call = new QListWidgetItem();
                call->setText(tr("Emergency Call"));
                call->setData(Qt::UserRole, "thewave:emergency call");
                call->setIcon(QIcon(":/icons/blank.svg"));
                QFont font = call->font();
                font.setPointSize(30);
                call->setFont(font);
                ui->listWidget->addItem(call);
                showtheWaveOption = false;
            } else if ((arg1.startsWith("call") && arg1.count() == 4) || arg1.startsWith("call ")) {
                QListWidgetItem *call = new QListWidgetItem();
                QString parse = arg1;
                if (arg1.count() == 4 || arg1.count() == 5) {
                    call->setText(tr("Place a call"));
                    call->setData(Qt::UserRole, "thewave:call");
                    call->setIcon(QIcon::fromTheme("call-start"));
                } else {
                    parse.remove(0, 5);
                    QListWidgetItem *callItem = new QListWidgetItem();
                    callItem->setText(tr("Place a call"));
                    callItem->setIcon(QIcon::fromTheme("call-start"));
                    callItem->setData(Qt::UserRole, "thewave:call " + parse);
                    ui->listWidget->addItem(callItem);

                    call->setText((QString) parse.at(0).toUpper() + parse.right(parse.length() - 1) + "");
                    call->setData(Qt::UserRole, "thewave:call " + parse);
                    call->setIcon(QIcon(":/icons/blank.svg"));

                    QFont font = call->font();
                    font.setPointSize(30);
                    call->setFont(font);
                }
                ui->listWidget->addItem(call);
                showtheWaveOption = false;
            } else if (arg1.startsWith("weather")) {
                QListWidgetItem *weather = new QListWidgetItem();

                QListWidgetItem *weatherItem = new QListWidgetItem();
                weatherItem->setText(tr("Weather"));
                weatherItem->setIcon(QIcon::fromTheme("weather-clear"));
                weatherItem->setData(Qt::UserRole, "thewave:weather");
                ui->listWidget->addItem(weatherItem);

                weather->setText(tr("Unknown"));
                weather->setData(Qt::UserRole, "thewave:weather");
                weather->setIcon(QIcon::fromTheme("dialog-error"));

                QFont font = weather->font();
                font.setPointSize(30);
                weather->setFont(font);

                ui->listWidget->addItem(weather);
                showtheWaveOption = false;
            }  else if (arg1.startsWith("play") && MainWin->isMprisAvailable()) {
                QListWidgetItem *i = new QListWidgetItem();
                i->setText(tr("Play"));
                i->setIcon(QIcon::fromTheme("media-playback-start"));
                i->setData(Qt::UserRole, "media:play");
                ui->listWidget->addItem(i);
                showtheWaveOption = false;
            } else if (arg1.startsWith("pause") && MainWin->isMprisAvailable()) {
                QListWidgetItem *i = new QListWidgetItem();
                i->setText(tr("Pause"));
                i->setIcon(QIcon::fromTheme("media-playback-pause"));
                i->setData(Qt::UserRole, "media:pause");
                ui->listWidget->addItem(i);
                showtheWaveOption = false;
            } else if (arg1.startsWith("next") && MainWin->isMprisAvailable()) {
                QListWidgetItem *i = new QListWidgetItem();
                i->setText(tr("Next Track"));
                i->setIcon(QIcon::fromTheme("media-skip-forward"));
                i->setData(Qt::UserRole, "media:next");
                ui->listWidget->addItem(i);
                showtheWaveOption = false;
            } else if ((arg1.startsWith("previous") || arg1.startsWith("back")) && MainWin->isMprisAvailable()) {
                QListWidgetItem *i = new QListWidgetItem();
                i->setText(tr("Previous Track"));
                i->setIcon(QIcon::fromTheme("media-skip-backward"));
                i->setData(Qt::UserRole, "media:previous");
                ui->listWidget->addItem(i);
                showtheWaveOption = false;
            } else if ((arg1.contains("current") || arg1.contains("what") || arg1.contains("now")) &&
                       (arg1.contains("track") || arg1.contains("song") || arg1.contains("playing")) && MainWin->isMprisAvailable()) { //Get current song info
                QListWidgetItem *nowPlaying = new QListWidgetItem();
                nowPlaying->setText(tr("Now Playing"));
                nowPlaying->setIcon(QIcon::fromTheme("media-playback-start"));
                nowPlaying->setData(Qt::UserRole, "thewave:current track");
                ui->listWidget->addItem(nowPlaying);

                QListWidgetItem *title = new QListWidgetItem();
                title->setText(MainWin->songName());
                title->setData(Qt::UserRole, "thewave:current track");
                title->setIcon(QIcon(":/icons/blank.svg"));
                QFont font = title->font();
                font.setPointSize(30);
                title->setFont(font);
                ui->listWidget->addItem(title);

                if (MainWin->songAlbum() != "" || MainWin->songArtist() != "") {
                    QListWidgetItem *extra = new QListWidgetItem();
                    if (MainWin->songArtist() != "" && MainWin->songAlbum() != "") {
                        extra->setText(MainWin->songArtist() + " · " + MainWin->songAlbum());
                    } else if (MainWin->songArtist() == "") {
                        extra->setText(MainWin->songAlbum());
                    } else {
                        extra->setText(MainWin->songArtist());
                    }
                    extra->setData(Qt::UserRole, "thewave:current track");
                    extra->setIcon(QIcon(":/icons/blank.svg"));
                    ui->listWidget->addItem(extra);
                }

                QListWidgetItem *space = new QListWidgetItem();
                ui->listWidget->addItem(space);

                QListWidgetItem *play = new QListWidgetItem();
                play->setText(tr("Play"));
                play->setIcon(QIcon::fromTheme("media-playback-start"));
                play->setData(Qt::UserRole, "media:play");
                ui->listWidget->addItem(play);

                QListWidgetItem *pause = new QListWidgetItem();
                pause->setText(tr("Pause"));
                pause->setIcon(QIcon::fromTheme("media-playback-pause"));
                pause->setData(Qt::UserRole, "media:pause");
                ui->listWidget->addItem(pause);

                QListWidgetItem *next = new QListWidgetItem();
                next->setText(tr("Next Track"));
                next->setIcon(QIcon::fromTheme("media-skip-forward"));
                next->setData(Qt::UserRole, "media:next");
                ui->listWidget->addItem(next);

                QListWidgetItem *prev = new QListWidgetItem();
                prev->setText(tr("Previous Track"));
                prev->setIcon(QIcon::fromTheme("media-skip-backward"));
                prev->setData(Qt::UserRole, "media:previous");
                ui->listWidget->addItem(prev);

                showtheWaveOption = false;
            }
        }*/

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

        if (showtheWaveOption && settings.value("thewave/enabled", true).toBool()) {
            App app;
            app.setCommand("thewave:" + query);
            app.setDescription(tr("Query theWave"));
            app.setName(query);
            app.setIcon(QIcon(":/icons/thewave.svg"));
            appsShown.append(app);
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

            App waveApp;
            waveApp.setCommand("thewave");
            waveApp.setIcon(QIcon(":/icons/thewave.svg"));
            waveApp.setName(tr("theWave"));
            waveApp.setDescription(tr("Personal Assistant"));
            apps.append(waveApp);

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
    return App::invalidApp();
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
        if (command.startsWith("thewave")) {
            if (command.split(":").count() > 1) {
                QStringList request = command.split(":");
                request.removeFirst();
                emit queryWave(request.join(" "));
            } else {
                emit queryWave("");
            }
            return false;
        } else if (command.startsWith("call:")) {
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
