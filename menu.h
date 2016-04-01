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

private:
    Ui::Menu *ui;

    bool checkFocus(QLayout *layout);

    QList<App*> *apps;
    QList<App*> *appsShown;

    bool doCheckForClose = false;

    //void closeEvent(QCloseEvent *event);
};

#endif // MENU_H
