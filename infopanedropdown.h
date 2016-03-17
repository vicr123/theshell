#ifndef INFOPANEDROPDOWN_H
#define INFOPANEDROPDOWN_H

#include <QDialog>
#include <QResizeEvent>

namespace Ui {
class InfoPaneDropdown;
}

class InfoPaneDropdown : public QDialog
{
    Q_OBJECT

public:
    explicit InfoPaneDropdown(QWidget *parent = 0);
    ~InfoPaneDropdown();

    enum dropdownType {
        Clock = 0,
        Battery = 1,
        Notifications = 2
    };

    void show(dropdownType showWith);
private slots:
    void on_pushButton_clicked();

private:
    Ui::InfoPaneDropdown *ui;

    dropdownType currentDropDown = Clock;
    void changeDropDown(dropdownType changeTo);

    void resizeEvent(QResizeEvent *event);
};

#endif // INFOPANEDROPDOWN_H
