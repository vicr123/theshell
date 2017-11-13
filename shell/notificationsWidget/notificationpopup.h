#ifndef NOTIFICATIONPOPUP_H
#define NOTIFICATIONPOPUP_H

#include <QDialog>
#include "tpropertyanimation.h"
#include "notificationsdbusadaptor.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QDirIterator>

namespace Ui {
class NotificationPopup;
}

class NotificationPopup : public QDialog
{
    Q_OBJECT

public:
    explicit NotificationPopup(int id, QWidget *parent = 0);
    ~NotificationPopup();

    void show();
    void close();

    void setApp(QString appName, QIcon appIcon);
    void setSummary(QString summary);
    void setBody(QString body);
    void setActions(QStringList actions);
    void setTimeout(int timeout);
    void setHints(QVariantMap hints);
    void setBigIcon(QIcon bigIcon);

private slots:
    void on_dismissButton_clicked();

signals:
    void actionClicked(QString key);
    void notificationClosed(uint reason);

private:
    Ui::NotificationPopup *ui;

    int id;
    int textHeight;
    bool mouseEvents = false;

    void enterEvent(QEvent* event);
    void leaveEvent(QEvent* event);
    void paintEvent(QPaintEvent* event);

    QTimer* dismisser = NULL;
    int timeoutLeft;
    QMap<QString, QString> actions;
    QVariantMap hints;

    static NotificationPopup* currentlyShowingPopup;
    static QList<NotificationPopup*> pendingPopups;
};

#endif // NOTIFICATIONPOPUP_H
