#ifndef BTHANDSFREE_H
#define BTHANDSFREE_H

#include <QWidget>
#include <QBoxLayout>
#include <QLine>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QApplication>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

class BTHandsfree : public QWidget
{
    Q_OBJECT
public:
    explicit BTHandsfree(QWidget *parent = 0);

signals:

public slots:
    void detectDevices();

private:
    QBoxLayout* layout;
    QLabel* infoLabel;
    QPushButton* hangupButton;
    QList<QDBusInterface*> interfaces;
    QDBusInterface* hangUpButtonInterface;
    QStringList knownDevices;
};

#endif // BTHANDSFREE_H
