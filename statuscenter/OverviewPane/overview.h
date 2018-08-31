#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QWidget>
#include <statuscenterpaneobject.h>
#include <QSettings>
#include <QGeoPositionInfoSource>
#include <QDBusObjectPath>
#include <QNetworkAccessManager>
#include <QApplication>

namespace Ui {
    class Overview;
}

struct BgObject {
    QPoint location;
    bool done = false;

    virtual void advance(int maxHeight, int maxWidth) = 0;
    virtual void paint(QPainter* p) = 0;
};

struct Raindrop : public BgObject {
    int velocity;
    int horizontalDisplacement;

    void advance(int maxHeight, int maxWidth);
    void paint(QPainter* p);
};

struct Aircraft : public BgObject {
    int velocity;

    void advance(int maxHeight, int maxWidth);
    void paint(QPainter* p);
};

struct Wind : public BgObject {
    float scale;
    int timeScale;
    int progress;

    void advance(int maxHeight, int maxWidth);
    void paint(QPainter *p);
};

struct WeatherCondition {
    Q_DECLARE_TR_FUNCTIONS(WeatherCondition)

public:
    WeatherCondition();
    WeatherCondition(int code);

    QString explanation;
    QIcon icon;

    bool isCloudy = false;
    bool isRainy = false;
    bool isSnowy = false;
    bool isWindy = false;
    bool isNull = true;
};

class Overview : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit Overview(QWidget *parent = nullptr);
        ~Overview();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    public slots:
        void updateDSTNotification();

        void launchDateTimeService();

        void updateWeather();

        void updateGeoclueLocation();

        void setAttribution(int attrib);

    private slots:
        void on_timersButton_toggled(bool checked);

    private:
        Ui::Overview *ui;

        bool eventFilter(QObject *watched, QEvent *event);
        void drawRaindrops(QPainter* p);
        void drawWind(QPainter* p);
        void drawObjects(QPainter* p);
        QTimer *animationTimer, *randomObjectTimer;

        QGeoPositionInfoSource* geolocationSource;
        QDBusObjectPath geoclueClientPath;
        QNetworkAccessManager networkMgr;
        bool weatherAvailable = false;

        WeatherCondition currentWeather;
        int currentYahooAttrib = 0;

        QList<Raindrop*> raindrops;
        QList<Wind*> winds;
        QList<BgObject*> objects;
        QSettings settings;

        QPixmap yahooAttribLight, yahooAttribDark;
};

#endif // OVERVIEW_H
