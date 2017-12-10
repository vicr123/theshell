#include "notificationswidget.h"
#include "ui_notificationswidget.h"

extern NotificationsDBusAdaptor* ndbus;
extern NativeEventFilter* NativeFilter;

NotificationsWidget::NotificationsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationsWidget)
{
    ui->setupUi(this);

    ndbus->setParentWidget(this);

    ui->scrollArea->installEventFilter(this);
}

NotificationsWidget::~NotificationsWidget()
{
    delete ui;
}

void NotificationsWidget::addNotification(NotificationObject *object) {
    notifications.insert(object->getId(), object);
    ui->noNotificationsFrame->setVisible(false);

    connect(object, &NotificationObject::actionClicked, [=](QString key) {
        emit ndbus->ActionInvoked(object->getId(), key);
        object->dismiss();
    });
    connect(object, &NotificationObject::closed, [=](NotificationObject::NotificationCloseReason reason) {
        emit ndbus->NotificationClosed(object->getId(), reason);
        notifications.remove(object->getId());

        if (notifications.count() == 0) {
            ui->noNotificationsFrame->setVisible(true);
        }
    });

    NotificationAppGroup* nGroup = NULL;

    for (NotificationAppGroup* group : notificationGroups) {
        if (group->getIdentifier() == object->getAppIdentifier()) {
            nGroup = group;
            break;
        }
    }

    if (nGroup == NULL) {
        nGroup = new NotificationAppGroup(object->getAppIdentifier(), object->getAppIcon(), object->getAppName());
        ((QBoxLayout*) ui->notificationGroups->layout())->insertWidget(0, nGroup);

        connect(nGroup, SIGNAL(notificationCountChanged()), this, SLOT(updateNotificationCount()));

        notificationGroups.append(nGroup);
    }

    nGroup->AddNotification(object);
}

bool NotificationsWidget::hasNotificationId(uint id) {
    return notifications.contains(id);
}

NotificationObject* NotificationsWidget::getNotification(uint id) {
    return notifications.value(id);
}

bool NotificationsWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == ui->scrollArea) {
        if (event->type() == QEvent::Resize) {
            ui->notificationGroups->setFixedWidth(ui->scrollArea->width());
        }
    }
    return false;
}

void NotificationsWidget::on_clearAllButton_clicked()
{
    for (NotificationAppGroup* group : notificationGroups) {
        group->clearAll();
    }
}

void NotificationsWidget::updateNotificationCount() {
    int count = 0;

    for (NotificationAppGroup* group : notificationGroups) {
        count += group->count();
    }

    emit numNotificationsChanged(count);
}


void NotificationsWidget::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
