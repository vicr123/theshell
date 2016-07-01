#ifndef CHOOSEBACKGROUND_H
#define CHOOSEBACKGROUND_H

#include <QDialog>
#include <QSvgRenderer>
#include "background.h"

namespace Ui {
class ChooseBackground;
}

class ChooseBackground : public QDialog
{
    Q_OBJECT

public:
    explicit ChooseBackground(QWidget *parent = 0);
    ~ChooseBackground();

signals:
    void reloadBackgrounds();

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

    void on_radioButton_2_toggled(bool checked);

    void on_radioButton_toggled(bool checked);

    void on_listWidget_currentRowChanged(int currentRow);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::ChooseBackground *ui;

    QIcon getSvgIcon(QString filename);
    QSettings settings;
};

#endif // CHOOSEBACKGROUND_H
