#ifndef OVERVIEW_H
#define OVERVIEW_H

#include <QWidget>
#include <statuscenterpaneobject.h>

namespace Ui {
    class Overview;
}

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
};

#endif // OVERVIEW_H
