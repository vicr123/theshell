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

#include "appslistmodel.h"
#include <qsettingsformats.h>
#include <application.h>

#include "mainwindow.h"

extern float getDPIScaling();
extern NativeEventFilter* NativeFilter;
extern MainWindow* MainWin;
extern void EndSession(EndSessionWait::shutdownType type);

struct AppsListModelPrivate {
    QSettings settings;
    QList<ApplicationPointer> apps;
    QList<ApplicationPointer> appsShown;
    bool queueLoadData = false;
    BTHandsfree* bt;

    QStringList pinnedAppsList;
};

AppsListModel::AppsListModel(QObject *parent) : QAbstractListModel(parent) {
    //this->bt = bt;
    d = new AppsListModelPrivate();

    connect(ApplicationDaemon::instance(), &ApplicationDaemon::appsUpdateRequired, this, [=] {
        loadData();
    });
    loadData();
}

AppsListModel::~AppsListModel() {
    delete d;
}

int AppsListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)

    return d->appsShown.count();
}

QVariant AppsListModel::data(const QModelIndex &index, int role) const {
    ApplicationPointer a = d->appsShown.at(index.row());
    if (d->appsShown.count() > index.row()) {
        if (role == Qt::DisplayRole) {
            return a->getProperty("Name", a->desktopEntry());
        } else if (role == Qt::DecorationRole) {
            return QIcon::fromTheme(a->getProperty("Icon").toString()).pixmap(QSize(32, 32) * theLibsGlobal::getDPIScaling());
        } else if (role == Qt::UserRole) { //Description
            return a->getProperty("GenericName", tr("Application"));
        } else if (role == Qt::UserRole + 1) { //Pinned
            return d->pinnedAppsList.contains(a->desktopEntry());
        } else if (role == Qt::UserRole + 2) { //Desktop Entry
            return a->desktopEntry();
        } else if (role == Qt::UserRole + 3) { //App
            return QVariant::fromValue(a);
        }
    }
    return QVariant();
}

void AppsListModel::updateData() {
    emit dataChanged(index(0), index(rowCount()));
}

void AppsListModel::search(QString query) {
    currentQuery = query;
    d->appsShown.clear();
    if (query == "") {
        d->appsShown.append(d->apps);
        updateData();
    } else {
        if (query.toLower().startsWith("call")) {
            QString number = query.mid(5);
            /*if (number != "" && this->bt != NULL) {
                QStringList devices = bt->getDevices();
                for (int i = 0; i < devices.count(); i++) {
                    App app;
                    app.setName(number);
                    app.setCommand("call:" + QString::number(i) + ":" + number);
                    app.setDescription(tr("Place a call over %1").arg(devices.at(i)));
                    app.setIcon(QIcon::fromTheme("call-start"));
                    appsShown.append(app);
                }
            }*/
        }

        int i = 0;
        for (ApplicationPointer app : d->apps) {
            if (i >= d->pinnedAppsList.count() || d->pinnedAppsList.count() == 0) {
                QStringList possibleWords;
                possibleWords.append(app->getProperty("Name").toString());
                possibleWords.append(app->getProperty("GenericName").toString());
                possibleWords.append(app->getStringList("Keywords"));

                for (QString s : possibleWords) {
                    if (s.contains(query, Qt::CaseInsensitive)) {
                        d->appsShown.append(app);
                        break;
                    }
                }
            }
            i++;
        }

        if (QString("shutdown").contains(query, Qt::CaseInsensitive) || QString("power off").contains(query, Qt::CaseInsensitive) ||  QString("shut down").contains(query, Qt::CaseInsensitive)) {
            d->appsShown.append(ApplicationPointer(new Application({
                {"Name", tr("Power Off")},
                {"Exec", "::poweroff"},
                {"GenericName", tr("Power off this device")},
                {"Icon", "system-shutdown"}
            })));
        } else if (QString("restart").contains(query, Qt::CaseInsensitive) || QString("reboot").contains(query, Qt::CaseInsensitive)) {
            d->appsShown.append(ApplicationPointer(new Application({
                {"Name", tr("Reboot")},
                {"Exec", "::reboot"},
                {"GenericName", tr("Reboot this device")},
                {"Icon", "system-reboot"}
            })));
        } else if (QString("logout").contains(query, Qt::CaseInsensitive) || QString("logoff").contains(query, Qt::CaseInsensitive)) {
            d->appsShown.append(ApplicationPointer(new Application({
                {"Name", tr("Log Out")},
                {"Exec", "::logout"},
                {"GenericName", tr("End your session")},
                {"Icon", "system-log-out"}
            })));
        }

        if (theLibsGlobal::searchInPath(query.split(" ")[0]).count() > 0) {
            d->appsShown.append(ApplicationPointer(new Application({
                {"Name", query},
                {"Exec", query},
                {"GenericName", tr("Run Command")},
                {"Icon", "system-run"}
            })));
        }

        QUrl uri = QUrl::fromUserInput(query);
        if (uri.scheme() == "http" || uri.scheme() == "https") {
            d->appsShown.append(ApplicationPointer(new Application({
                {"Name", uri.toDisplayString()},
                {"Exec", "xdg-open \"" + uri.toString() + "\""},
                {"GenericName", tr("Open webpage")},
                {"Icon", "text-html"}
            })));
        } else if (uri.scheme() == "file") {
            if (QDir(uri.path() + "/").exists()) {
                d->appsShown.append(ApplicationPointer(new Application({
                    {"Name", uri.path()},
                    {"Exec", "xdg-open \"" + uri.toString() + "\""},
                    {"GenericName", tr("Open Folder")},
                    {"Icon", "system-file-manager"}
                })));
            } else if (QFile(uri.path()).exists()) {
                QFileInfo info(uri.toLocalFile());
                QMimeType mime = QMimeDatabase().mimeTypeForFile(info);

                d->appsShown.append(ApplicationPointer(new Application({
                    {"Name", uri.path()},
                    {"Exec", "xdg-open \"" + uri.toString() + "\""},
                    {"GenericName", tr("Open File")},
                    {"Icon", mime.iconName()}
                })));
            }
        }

        updateData();
    }
}

void AppsListModel::loadData() {
    d->pinnedAppsList.clear();
    d->apps.clear();

    QList<ApplicationPointer> normalApps;
    for (QString desktopEntry : Application::allApplications()) {
        ApplicationPointer a(new Application(desktopEntry));

        //Make sure this app is good to be shown
        if (a->getProperty("Type", "").toString() != "Application") continue;
        if (a->getProperty("NoDisplay", false).toBool()) continue;
        if (!a->getStringList("OnlyShowIn", {"theshell"}).contains("theshell")) continue;
        if (a->getStringList("NotShowIn").contains("theshell")) continue;
        normalApps.append(a);
    }
    normalApps.append(ApplicationPointer(new Application({
        {"Name", tr("System settings")},
        {"Exec", "::settings"},
        {"GenericName", tr("System Configuration")},
        {"Icon", "configure"}
    })));

    std::sort(normalApps.begin(), normalApps.end(), [](const ApplicationPointer& a, const ApplicationPointer& b) -> bool {
        if (a->getProperty("Name").toString().localeAwareCompare(b->getProperty("Name").toString()) < 0) {
            return true;
        } else {
            return false;
        }
    });

    //Add in pinned apps
    QList<ApplicationPointer> pinnedApps;
    d->settings.beginGroup("gateway");
    int count = d->settings.beginReadArray("pinnedItems");
    for (int i = 0; i < count; i++) {
        d->settings.setArrayIndex(i);

        QString desktopEntry = d->settings.value("desktopEntry").toString();
        if (desktopEntry.startsWith("/")) {
            QFileInfo file(desktopEntry);
            desktopEntry = file.completeBaseName();
        }

        d->pinnedAppsList.append(desktopEntry);

        pinnedApps.append(ApplicationPointer(new Application(desktopEntry)));
    }
    d->settings.endArray();
    d->settings.endGroup();

    d->apps.append(pinnedApps);
    d->apps.append(normalApps);

    //Perform a search to initialize the list
    search(currentQuery);
}

AppsDelegate::AppsDelegate(QWidget *parent, bool drawArrows) : QStyledItemDelegate(parent) {
    this->drawArrows = drawArrows;
}

void AppsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->setFont(option.font);
    painter->setLayoutDirection(option.direction);

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

        if (option.direction == Qt::RightToLeft) {
            iconRect.moveRight(option.rect.right() - 32 * getDPIScaling());
            textRect.moveRight(option.rect.right() - 6 * getDPIScaling());
        }

        if (option.state & QStyle::State_Selected) {
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::Highlight));
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::HighlightedText));
            painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
        } else if (option.state & QStyle::State_MouseOver) {
            QColor col = option.palette.color(QPalette::Highlight);
            col.setAlpha(127);
            painter->setBrush(col);
            painter->setPen(Qt::transparent);
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
        } else {
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
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

        if (option.direction == Qt::RightToLeft) {
            iconRect.moveRight(option.rect.right() - 6 * getDPIScaling());
            textRect.moveRight(iconRect.left() - 6 * getDPIScaling());
            descRect.moveRight(iconRect.left() - 6 * getDPIScaling());
        }

        if (option.state & QStyle::State_Selected) {
            painter->setPen(Qt::transparent);
            painter->setBrush(option.palette.color(QPalette::Highlight));
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::HighlightedText));
            painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
            painter->drawText(descRect, Qt::AlignLeading, index.data(Qt::UserRole).toString());
        } else if (option.state & QStyle::State_MouseOver) {
            QColor col = option.palette.color(QPalette::Highlight);
            col.setAlpha(127);
            painter->setBrush(col);
            painter->setPen(Qt::transparent);
            painter->drawRect(option.rect);
            painter->setBrush(Qt::transparent);
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(descRect, Qt::AlignLeading, index.data(Qt::UserRole).toString());
        } else {
            painter->setPen(option.palette.color(QPalette::WindowText));
            painter->drawText(textRect, Qt::AlignLeading, index.data().toString());
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::WindowText));
            painter->drawText(descRect, Qt::AlignLeading, index.data(Qt::UserRole).toString());
        }
    }
    painter->drawPixmap(iconRect, index.data(Qt::DecorationRole).value<QPixmap>());

    if (drawArrows) {
        ApplicationPointer a = index.data(Qt::UserRole + 3).value<ApplicationPointer>();
        if (a->getStringList("Actions").count() > 0) { //Actions included
            QRect actionsRect;
            actionsRect.setWidth(16 * getDPIScaling());
            actionsRect.setHeight(16 * getDPIScaling());
            actionsRect.moveRight(option.rect.right() - 9 * getDPIScaling());
            actionsRect.moveTop(option.rect.top() + option.rect.height() / 2 - 8 * getDPIScaling());

            if (option.direction == Qt::RightToLeft) {
                actionsRect.moveLeft(option.rect.left() + 9 * getDPIScaling());
            }

            painter->drawPixmap(actionsRect, QIcon::fromTheme("arrow-right").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
        }
    }

    int pinned = ((AppsListModel*) index.model())->pinnedAppsCount();
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
    ApplicationPointer app = d->appsShown.at(index.row());
    QString command = app->getProperty("Exec").toString().remove("%u");
    /*if (command.startsWith("call:")) {
        QString callCommand = command.mid(5);
        QStringList parts = callCommand.split(":");
        int deviceIndex = parts.at(0).toInt();
        QString number = parts.at(1);
        bt->placeCall(deviceIndex, number);
        return true;
    } else */if (command == "::settings") {
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

        commandSpace.removeAll("%f");
        commandSpace.removeAll("%F");
        commandSpace.removeAll("%u");
        commandSpace.removeAll("%U");
        if (commandSpace.contains("%i")) {
            if (app->hasProperty("Icon")) {
                commandSpace.replaceInStrings("%i", "--icon " + app->getProperty("Icon").toString());
            } else {
                commandSpace.removeAll("%i");
            }
        }
        commandSpace.replaceInStrings("%c", app->getProperty("Name").toString());
        commandSpace.replaceInStrings("%%", "%");

        QString finalCommand = commandSpace.join(" ");
        qDebug() << "Starting command:" << finalCommand;
        process->start(finalCommand);
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=](int exitCode, QProcess::ExitStatus exitStatus){
            process->deleteLater();
        });
        connect(process, &QProcess::readyReadStandardError, [=] {
            process->readAllStandardError(); //Discard stderr
        });
        connect(process, &QProcess::readyReadStandardOutput, [=] {
            process->readAllStandardOutput(); //Discard stdout
        });
        return process->waitForStarted();
    }
}

int AppsListModel::pinnedAppsCount() {
    return d->pinnedAppsList.count();
}
