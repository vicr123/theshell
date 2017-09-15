#include "notificationswidget.h"
#include "ui_notificationswidget.h"

extern NotificationsDBusAdaptor* ndbus;

NotificationsWidget::NotificationsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationsWidget)
{
    ui->setupUi(this);

    ndbus->setParentWidget(this);
}

NotificationsWidget::~NotificationsWidget()
{
    delete ui;
}

void NotificationsWidget::addNotification(NotificationObject *object) {
    notifications.insert(object->getId(), object);

    connect(object, &NotificationObject::actionClicked, [=](QString key) {
        emit ndbus->ActionInvoked(object->getId(), key);
    });
    connect(object, &NotificationObject::closed, [=](NotificationObject::NotificationCloseReason reason) {
        emit ndbus->NotificationClosed(object->getId(), reason);
    });
}

bool NotificationsWidget::hasNotificationId(uint id) {
    return notifications.contains(id);
}

NotificationObject* NotificationsWidget::getNotification(uint id) {
    return notifications.value(id);
}
