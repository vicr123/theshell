#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QWidget>
#include <statuscenterpaneobject.h>

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

    private:
        Ui::Overview *ui;

        bool eventFilter(QObject *watched, QEvent *event);
        void drawRaindrops(QPainter* p);
        void drawObjects(QPainter* p);
        QTimer *animationTimer, *randomObjectTimer;

        QList<Raindrop*> raindrops;
        QList<BgObject*> objects;
};

#endif // OVERVIEW_H
