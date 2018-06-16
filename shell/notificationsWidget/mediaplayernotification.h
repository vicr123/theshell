#ifndef MEDIAPLAYERNOTIFICATION_H
#define MEDIAPLAYERNOTIFICATION_H

#include <QFrame>
#include <QDBusConnection>
#include <QDBusReply>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QLabel>
#include <QIcon>
#include <QPushButton>
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>

namespace Ui {
    class MediaPlayerNotification;
}

class MediaPlayerNotification : public QFrame
{
        Q_OBJECT

    public:
        explicit MediaPlayerNotification(QString service, QWidget *parent = 0);
        ~MediaPlayerNotification();

    public slots:
        void updateMpris(QString interfaceName, QMap<QString, QVariant> properties, QStringList changedProperties);
        void setDetails(QString title, QString artist, QString album, QString albumArt);

    private slots:
        void on_backButton_clicked();

        void on_playPauseButton_clicked();

        void on_nextButton_clicked();

        void on_closeButton_clicked();

    private:
        Ui::MediaPlayerNotification *ui;
        QString service;
        QNetworkAccessManager mgr;
};

#endif // MEDIAPLAYERNOTIFICATION_H
