#include "notificationpanel.h"
#include "ui_notificationpanel.h"

NotificationPanel::NotificationPanel(NotificationObject* object, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NotificationPanel)
{
    ui->setupUi(this);

    this->object = object;
    connect(object, SIGNAL(parametersUpdated()), this, SLOT(updateParameters()));
    connect(object, SIGNAL(closed(NotificationObject::NotificationCloseReason)), this, SLOT(closeNotification(NotificationObject::NotificationCloseReason)));

    ui->bodyLabel->setFixedHeight(ui->bodyLabel->fontMetrics().height());

    updateParameters();
    this->setFixedHeight(0);
}

NotificationPanel::~NotificationPanel()
{
    delete ui;
}

void NotificationPanel::mouseReleaseEvent(QMouseEvent *event) {
    this->toggleExpandNormal();
}

void NotificationPanel::updateParameters() {
    ui->summaryLabel->setText(object->getSummary());
    ui->bodyLabel->setText(object->getBody());
}

void NotificationPanel::on_closeButton_clicked()
{
    object->dismiss();
}

void NotificationPanel::closeNotification(NotificationObject::NotificationCloseReason reason) {
    if (reason == NotificationObject::Dismissed) {
        //Close notification
        emit dismissed();

        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(this->height());
        anim->setEndValue(0);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setDuration(500);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            this->setFixedHeight(value.toInt());
        });
        connect(anim, SIGNAL(finished()), this, SLOT(deleteLater()));
        anim->start();
    }
}

NotificationObject* NotificationPanel::getObject() {
    return this->object;
}

void NotificationPanel::collapseHide() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->height());
    anim->setEndValue(0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(stop()));
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(deleteLater()));
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    anim->start();
}

void NotificationPanel::expandHide() {
    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(this->height());
    anim->setEndValue(this->sizeHint().height());
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->setDuration(500);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(stop()));
    connect(this, SIGNAL(destroyed(QObject*)), anim, SLOT(deleteLater()));
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        this->setFixedHeight(value.toInt());
    });
    connect(anim, &tVariantAnimation::finished, [=] {
        //Remove layout constraints
        this->setFixedSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    });
    anim->start();
}

void NotificationPanel::toggleExpandNormal() {
    expanded = !expanded;

    if (expanded) {
        int textHeight = ui->bodyLabel->fontMetrics().boundingRect(QRect(0, 0, ui->bodyLabel->width(), 10000), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, ui->bodyLabel->text()).height();

        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(ui->bodyLabel->height());
        anim->setEndValue(textHeight);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setDuration(500);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->bodyLabel->setFixedHeight(value.toInt());
        });
        anim->start();
    } else {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(ui->bodyLabel->height());
        anim->setEndValue(ui->bodyLabel->fontMetrics().height());
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->setDuration(500);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->bodyLabel->setFixedHeight(value.toInt());
        });
        anim->start();
    }
}
