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
#include <QTimer>
#include <QSlider>

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

        void updatePosition();

        void updatePosition(qint64 position);

        void on_position_valueChanged(int value);

    private:
        Ui::MediaPlayerNotification *ui;
        QString service;
        QNetworkAccessManager mgr;
        double rate;
        QString playbackStatus;
        QDBusObjectPath trackId;

        QPalette defaultPal;
};

#endif // MEDIAPLAYERNOTIFICATION_H
