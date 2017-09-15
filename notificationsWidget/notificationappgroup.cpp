#include "notificationappgroup.h"
#include "ui_notificationappgroup.h"

extern float getDPIScaling();

NotificationAppGroup::NotificationAppGroup(QString appIdentifier, QIcon appIcon, QString appName, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::NotificationAppGroup)
{
    ui->setupUi(this);

    this->appIcon = appIcon;
    this->appIdentifier = appIdentifier;

    ui->appIcon->setPixmap(appIcon.pixmap(24 * getDPIScaling(), 24 * getDPIScaling()));
    ui->appName->setText(appName);

    //this->setFixedHeight(0);
}

NotificationAppGroup::~NotificationAppGroup()
{
    delete ui;
}

QString NotificationAppGroup::getIdentifier() {
    return this->appIdentifier;
}

void NotificationAppGroup::AddNotification(NotificationObject *object) {
    NotificationPanel* panel = new NotificationPanel(object);
    ((QBoxLayout*) ui->notificationsWidget->layout())->addWidget(panel);

    connect(panel, &NotificationPanel::dismissed, [=] {
        notifications.removeAll(panel);

        if (notifications.count() == 0) {
            tVariantAnimation* anim = new tVariantAnimation();
            anim->setStartValue(this->height());
            anim->setEndValue(0);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            anim->setDuration(500);
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                this->setFixedHeight(value.toInt());
            });
            anim->start();
        }
    });

    QTimer::singleShot(100, [=] {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(this->height());
        anim->setEndValue(this->sizeHint().height());
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setDuration(500);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            this->setFixedHeight(value.toInt());
        });
        connect(anim, &tVariantAnimation::finished, [=] {
            //Remove layout constraints
            this->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        });
        anim->start();
    });

    notifications.append(panel);
}

int NotificationAppGroup::count() {
    return notifications.count();
}


void NotificationAppGroup::on_closeAllNotificationsButton_clicked()
{
    while (!notifications.isEmpty()) {
        notifications.takeFirst()->getObject()->dismiss();
    }
}
