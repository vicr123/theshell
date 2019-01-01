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
    virtual ~BgObject();

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

        void updateGeoclueLocation(double latitude, double longitude);

        void setAttribution(int attrib);

    private slots:
        void on_timersButton_toggled(bool checked);

        void on_stopwatchButton_toggled(bool checked);

        void on_remindersButton_toggled(bool checked);

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

        void changeEvent(QEvent* event);

        QPixmap yahooAttribLight, yahooAttribDark;
};

#endif // OVERVIEW_H
