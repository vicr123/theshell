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

#include "appslistmodel.h"

extern float getDPIScaling();
extern NativeEventFilter* NativeFilter;
extern MainWindow* MainWin;
extern QSettings::Format desktopFileFormat;
extern void EndSession(EndSessionWait::shutdownType type);

AppsListModel::AppsListModel(BTHandsfree* bt, QObject *parent) : QAbstractListModel(parent) {
    this->bt = bt;
    loadData();
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

        if (QString("shutdown").contains(query, Qt::CaseInsensitive) || QString("power off").contains(query, Qt::CaseInsensitive) ||  QString("shut down").contains(query, Qt::CaseInsensitive)) {
            App app;
            app.setName(tr("Power Off"));
            app.setCommand("::poweroff");
            app.setDescription(tr("Power off this device"));
            app.setIcon(QIcon::fromTheme("system-shutdown"));

            /*App reboot;
            reboot.setName(tr("Reboot"));
            reboot.setCommand("::reboot");
            reboot.setIcon(QIcon::fromTheme("system-reboot"));
            app.addAction(reboot);

            App logout;
            logout.setName(tr("Log Out"));
            logout.setCommand("::logout");
            logout.setIcon(QIcon::fromTheme("system-log-out"));
            app.addAction(logout);*/

            appsShown.append(app);
        } else if (QString("restart").contains(query, Qt::CaseInsensitive) || QString("reboot").contains(query, Qt::CaseInsensitive)) {
            App app;
            app.setName(tr("Reboot"));
            app.setCommand("::reboot");
            app.setDescription(tr("Reboot this device"));
            app.setIcon(QIcon::fromTheme("system-reboot"));
            appsShown.append(app);
        } else if (QString("logout").contains(query, Qt::CaseInsensitive) || QString("logoff").contains(query, Qt::CaseInsensitive)) {
            App app;
            app.setName(tr("Log Out"));
            app.setCommand("::logout");
            app.setDescription(tr("End your session"));
            app.setIcon(QIcon::fromTheme("system-log-out"));
            appsShown.append(app);
        }

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
            settingsApp.setCommand("::settings");
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

    if (desc.contains("GenericName[" + lang + "]")) {
        app.setDescription(desc.value("GenericName[" + lang + "]").toString());
    } else {
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
        QStringList availableActions = desc.value("Actions").toString().split(";", QString::SkipEmptyParts);
        desc.endGroup();

        for (QString action : availableActions) {
            desc.beginGroup("Desktop Action " + action);

            App act;
            if (desc.contains("Name[" + lang + "]")) {
                act.setName(desc.value("Name[" + lang + "]").toString());
            } else {
                act.setName(desc.value("Name").toString());
            }

            QString commandLine = desc.value("Exec").toString();
            commandLine.remove("%u");
            commandLine.remove("%U");
            commandLine.remove("%f");
            commandLine.remove("%F");
            commandLine.remove("%k");
            commandLine.remove("%i");
            commandLine.replace("%c", "\"" + app.name() + "\"");
            act.setCommand(commandLine);

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

AppsDelegate::AppsDelegate(QWidget *parent, bool drawArrows) : QStyledItemDelegate(parent) {
    this->drawArrows = drawArrows;
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

    if (drawArrows) {
        App a = index.data(Qt::UserRole + 3).value<App>();
        if (a.actions().count() > 0) { //Actions included
            QRect actionsRect;
            actionsRect.setWidth(16 * getDPIScaling());
            actionsRect.setHeight(16 * getDPIScaling());
            actionsRect.moveRight(option.rect.right() - 9 * getDPIScaling());
            actionsRect.moveTop(option.rect.top() + option.rect.height() / 2 - 8 * getDPIScaling());

            painter->drawPixmap(actionsRect, QIcon::fromTheme("arrow-right").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
        }
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
    QString command = appsShown.at(index.row()).command().remove("%u");
    if (command.startsWith("call:")) {
        QString callCommand = command.mid(5);
        QStringList parts = callCommand.split(":");
        int deviceIndex = parts.at(0).toInt();
        QString number = parts.at(1);
        bt->placeCall(deviceIndex, number);
        return true;
    } else if (command == "::settings") {
        MainWin->getInfoPane()->show(InfoPaneDropdown::Settings);
        return true;
    } else if (command == "::poweroff") {
        EndSession(EndSessionWait::powerOff);
        return true;
    } else if (command == "::reboot") {
        EndSession(EndSessionWait::reboot);
        return true;
    } else if (command == "::logout") {
        EndSession(EndSessionWait::logout);
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
}
