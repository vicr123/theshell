#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QWidget>
#include <statuscenterpaneobject.h>

namespace Ui {
    class Overview;
}

struct Raindrop {
    QPoint location;
    int velocity;
    int horizontalDisplacement;

    bool advance(int maxHeight);
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

        QList<Raindrop*> raindrops;
};

#endif // OVERVIEW_H
