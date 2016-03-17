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
#include "endsessionwait.h"
#include "app.h"

namespace Ui {
class Menu;
}

class Menu : public QDialog
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = 0);
    ~Menu();

    void show();

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

private:
    Ui::Menu *ui;

    bool checkFocus(QLayout *layout);

    QList<App*> *apps;
    QList<App*> *appsShown;
};

#endif // MENU_H
