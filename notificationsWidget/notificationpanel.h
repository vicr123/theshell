#ifndef NOTIFICATIONPANEL_H
#define NOTIFICATIONPANEL_H

#include <QWidget>
#include <QMouseEvent>
#include "notificationobject.h"
#include "tpropertyanimation.h"

namespace Ui {
class NotificationPanel;
}

class NotificationPanel : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationPanel(NotificationObject* object, QWidget *parent = 0);
    ~NotificationPanel();

    NotificationObject* getObject();

private slots:
    void updateParameters();

    void closeNotification(NotificationObject::NotificationCloseReason reason);

    void on_closeButton_clicked();

signals:
    void dismissed();

private:
    Ui::NotificationPanel *ui;

    bool expanded = false;

    void mouseReleaseEvent(QMouseEvent* event);

    NotificationObject* object;
};

#endif // NOTIFICATIONPANEL_H
