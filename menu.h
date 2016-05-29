#ifndef MENU_H
#define MENU_H

#include <QDialog>
#include <QFocusEvent>
#include <QTimer>
#include <QPropertyAnimation>
#include <QDir>
#include <QList>
#include <QListWidgetItem>
#include <QDirIterator>
#include <QPaintEvent>
#include <QPainter>
#include <QMimeType>
#include <QMimeDatabase>
#include <systemd/sd-login.h>
#include "endsessionwait.h"
#include "app.h"
#include "mainwindow.h"

#undef KeyPress

namespace Ui {
class Menu;
}

class Menu : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)

public:
    explicit Menu(QWidget *parent = 0);
    ~Menu();
    void setGeometry(int x, int y, int w, int h);
    void setGeometry(QRect geometry);

    void show();
    void close();

signals:
    void appOpening(QString name, QIcon icon);
    void menuClosing();

private slots:
    void checkForclose();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_commandLinkButton_clicked();

    void on_commandLinkButton_2_clicked();

    void on_commandLinkButton_3_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_textEdited(const QString &arg1);

    bool eventFilter(QObject *object, QEvent *event);

    void on_lineEdit_returnPressed();

    void on_pushButton_3_clicked();

    void on_commandLinkButton_5_clicked();

    void on_commandLinkButton_7_clicked();

    void on_commandLinkButton_8_clicked();

private:
    Ui::Menu *ui;

    bool checkFocus(QLayout *layout);

    QList<App*> *apps;
    QList<App*> *appsShown;

    bool doCheckForClose = false;

    //void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent* event);
    void changeEvent(QEvent* event);
};

#endif // MENU_H
