#ifndef APPSLISTMODEL_H
#define APPSLISTMODEL_H

#include <QAbstractListModel>
#include <QFuture>
#include <QStyleOptionViewItem>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QProcessEnvironment>
#include <QStyledItemDelegate>
#include <QUrl>
#include <QSettings>
#include <QFontInfo>
#include <QMimeType>
#include <QMimeDatabase>
#include <QtConcurrent>
#include <QPainter>
#include <QListView>
#include "bthandsfree.h"
#include "app.h"
#include "nativeeventfilter.h"

class AppsListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    AppsListModel(BTHandsfree* bt = NULL, QObject *parent = 0);
    ~AppsListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void updateData();
    int pinnedAppsCount;
    bool launchApp(QModelIndex index);
    void search(QString query);

    QList<App> availableApps();

    QString currentQuery = "";

public slots:
    void loadData();

signals:
    void queryWave(QString query);

private:
    struct dataLoad {
        QList<App> apps;
        int pinnedAppsCount;
    };

    App readAppFile(QString appFile, QStringList pinnedAppsList);

    QSettings settings;
    QList<App> apps;
    QList<App> appsShown;
    QFuture<dataLoad> loadDataFuture;
    bool queueLoadData = false;
    BTHandsfree* bt;
};

class AppsDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    AppsDelegate(QWidget *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // APPSLISTMODEL_H
