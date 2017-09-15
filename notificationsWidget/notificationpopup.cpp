#include "notificationpopup.h"
#include "ui_notificationpopup.h"

#include "notificationobject.h"

extern float getDPIScaling();
extern NotificationsDBusAdaptor* ndbus;

NotificationPopup::NotificationPopup(int id, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NotificationPopup)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_X11NetWmWindowTypeNotification, true);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    ui->buttonsWidget->setFixedHeight(0);
    ui->downArrow->setPixmap(QIcon::fromTheme("go-down").pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
    ui->ContentsWidget->setFixedHeight(ui->bodyLabel->fontMetrics().height() + ui->ContentsWidget->layout()->contentsMargins().top());
    this->id = id;

    dismisser = new QTimer();
    dismisser->setInterval(500);
    connect(dismisser, &QTimer::timeout, [=] {
        if (timeoutLeft != -2000) {
            timeoutLeft -= 500;
            if (timeoutLeft < 0) {
                this->close();
                emit notificationClosed(NotificationObject::Expired);
            }
        }
    });
}

NotificationPopup::~NotificationPopup()
{
    delete ui;
}

void NotificationPopup::show() {
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    this->move(screenGeometry.topLeft().x(), screenGeometry.top() - this->height());
    this->setFixedWidth(screenGeometry.width());

    textHeight = ui->bodyLabel->fontMetrics().boundingRect(QRect(0, 0, screenGeometry.width() - this->layout()->contentsMargins().left() - this->layout()->contentsMargins().right(), 10000), Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, ui->bodyLabel->text()).height();

    bool showDownArrow = false;
    if (textHeight > ui->bodyLabel->fontMetrics().height()) {
        showDownArrow = true;
    }
    if (actions.count() > 0) {
        showDownArrow = true;
    }
    ui->downContainer->setVisible(showDownArrow);

    this->setFixedHeight(this->sizeHint().height());
    QDialog::show();

    tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(this->x(), screenGeometry.y(), this->width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();

    dismisser->start();

    mouseEvents = true;
}

void NotificationPopup::close() {
    dismisser->stop();

    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    tPropertyAnimation* anim = new tPropertyAnimation(this, "geometry");
    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(this->x(), screenGeometry.y() - this->height(), this->width(), this->height()));
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    connect(anim, &tPropertyAnimation::finished, [=] {
        QDialog::close();
    });
    anim->start();

    mouseEvents = false;
}

void NotificationPopup::enterEvent(QEvent *event) {
    Q_UNUSED(event)

    if (mouseEvents) {
        tVariantAnimation* anim = new tVariantAnimation();
        anim->setStartValue(ui->buttonsWidget->height());
        anim->setEndValue(ui->buttonsWidget->sizeHint().height());
        anim->setDuration(250);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->buttonsWidget->setFixedHeight(value.toInt());
            this->setFixedHeight(this->sizeHint().height());
        });
        anim->start();

        tVariantAnimation* anim2 = new tVariantAnimation();
        anim2->setStartValue(ui->downContainer->height());
        anim2->setEndValue(0);
        anim2->setDuration(250);
        anim2->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim2, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim2, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->downContainer->setFixedHeight(value.toInt());
        });
        anim2->start();

        tVariantAnimation* anim3 = new tVariantAnimation();
        anim3->setStartValue(ui->ContentsWidget->height());
        anim3->setEndValue(textHeight);
        anim3->setDuration(250);
        anim3->setEasingCurve(QEasingCurve::OutCubic);
        connect(anim3, SIGNAL(finished()), anim, SLOT(deleteLater()));
        connect(anim3, &tVariantAnimation::valueChanged, [=](QVariant value) {
            ui->ContentsWidget->setFixedHeight(value.toInt() + ui->ContentsWidget->layout()->contentsMargins().top());
        });
        anim3->start();

        dismisser->stop();
    }
}

void NotificationPopup::leaveEvent(QEvent *event) {
    Q_UNUSED(event)

    if (mouseEvents) {
        if (!this->rect().contains(mapFromGlobal(QCursor::pos()))) {
            tVariantAnimation* anim = new tVariantAnimation();
            anim->setStartValue(ui->buttonsWidget->height());
            anim->setEndValue(0);
            anim->setDuration(250);
            anim->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->buttonsWidget->setFixedHeight(value.toInt());
                this->setFixedHeight(this->sizeHint().height());
            });
            anim->start();

            tVariantAnimation* anim2 = new tVariantAnimation();
            anim2->setStartValue(ui->downContainer->height());
            anim2->setEndValue(ui->downContainer->sizeHint().height());
            anim2->setDuration(250);
            anim2->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim2, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim2, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->downContainer->setFixedHeight(value.toInt());
            });
            anim2->start();

            tVariantAnimation* anim3 = new tVariantAnimation();
            anim3->setStartValue(ui->ContentsWidget->height());
            anim3->setEndValue(ui->bodyLabel->fontMetrics().height());
            anim3->setDuration(250);
            anim3->setEasingCurve(QEasingCurve::OutCubic);
            connect(anim3, SIGNAL(finished()), anim, SLOT(deleteLater()));
            connect(anim3, &tVariantAnimation::valueChanged, [=](QVariant value) {
                ui->ContentsWidget->setFixedHeight(value.toInt() + ui->ContentsWidget->layout()->contentsMargins().top());
            });
            anim3->start();

            dismisser->start();
        }
    }
}

void NotificationPopup::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(this->palette().color(QPalette::WindowText));
    painter.drawLine(0, this->height() - 1, this->width(), this->height() - 1);
}

void NotificationPopup::setApp(QString appName, QIcon appIcon) {
    ui->appnameLabel->setText(appName);
    ui->appIcon->setPixmap(appIcon.pixmap(16 * getDPIScaling(), 16 * getDPIScaling()));
}

void NotificationPopup::setSummary(QString summary) {
    ui->summaryLabel->setText(summary);
}

void NotificationPopup::setBody(QString body) {
    ui->bodyLabel->setText(body);
}

void NotificationPopup::setHints(QVariantMap hints) {
    this->hints = hints;
}

void NotificationPopup::setActions(QStringList actions) {
    if (actions.count() % 2 == 0) {
        for (int i = 0; i < actions.length(); i += 2) {
            QString key = actions.at(i);
            QString value = actions.at(i + 1);

            QPushButton* button = new QPushButton();
            button->setText(value);
            connect(button, &QPushButton::clicked, [=] {
                emit actionClicked(key);
                this->close();
            });
            ((QBoxLayout*) ui->actionsWidget->layout())->addWidget(button);

            this->actions.insert(key, value);
        }
    }
}

void NotificationPopup::setTimeout(int timeout) {
    if (timeout == 0) {
        timeoutLeft = -2000;
    } else {
        timeoutLeft = timeout;
    }
}

void NotificationPopup::on_dismissButton_clicked()
{
    this->close();
    emit notificationClosed(NotificationObject::Dismissed);
}

void NotificationPopup::setBigIcon(QIcon bigIcon) {
    ui->bigIconLabel->setPixmap(bigIcon.pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
}
