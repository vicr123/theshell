#include "choosebackground.h"
#include "ui_choosebackground.h"

ChooseBackground::ChooseBackground(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChooseBackground)
{
    ui->setupUi(this);

    QString backPath = settings.value("desktop/background", "inbuilt:triangles").toString();

    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/triangles"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/blueprint"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/triplecircle"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/shatter"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/slice"), ""));
    ui->listWidget->addItem(new QListWidgetItem(getSvgIcon(":/backgrounds/nav"), ""));

    if (backPath.startsWith("inbuilt:")) { //Inbuilt background
        ui->radioButton->setChecked(true);
        QString selection = ":/backgrounds/" + backPath.split(":").at(1);
        if (selection == "triangles") {
            ui->listWidget->item(0)->setSelected(true);
        } else if (selection == "blueprint") {
            ui->listWidget->item(1)->setSelected(true);
        } else if (selection == "triplecircle") {
            ui->listWidget->item(2)->setSelected(true);
        } else if (selection == "shatter") {
            ui->listWidget->item(3)->setSelected(true);
        } else if (selection == "slice") {
            ui->listWidget->item(4)->setSelected(true);
        } else if (selection == "nav") {
            ui->listWidget->item(5)->setSelected(true);
        }
        ui->lineEdit->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    } else {
        ui->radioButton_2->setChecked(true);
        ui->lineEdit->setText(backPath);
        ui->listWidget->setEnabled(false);
    }

}

ChooseBackground::~ChooseBackground()
{
    delete ui;
}

QIcon ChooseBackground::getSvgIcon(QString filename) {
    QPixmap background(150, 84);
    QSvgRenderer renderer(filename);
    QPainter painter(&background);
    renderer.render(&painter, background.rect());
    return QIcon(background);
}

void ChooseBackground::on_lineEdit_textChanged(const QString &arg1)
{
    settings.setValue("desktop/background", arg1);
    emit reloadBackgrounds();
    this->setFocus();
}

void ChooseBackground::on_radioButton_2_toggled(bool checked)
{
    if (checked) {
        ui->lineEdit->setEnabled(true);
        ui->pushButton_2->setEnabled(true);
    } else {
        ui->lineEdit->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    }
}

void ChooseBackground::on_radioButton_toggled(bool checked)
{
    if (checked) {
        ui->listWidget->setEnabled(true);
    } else {
        ui->listWidget->setEnabled(false);
    }
}

void ChooseBackground::on_listWidget_currentRowChanged(int currentRow)
{
    switch (currentRow) {
    case 0:
        settings.setValue("desktop/background", "inbuilt:triangles");
        emit reloadBackgrounds();
        break;
    case 1:
        settings.setValue("desktop/background", "inbuilt:blueprint");
        emit reloadBackgrounds();
        break;
    case 2:
        settings.setValue("desktop/background", "inbuilt:triplecircle");
        emit reloadBackgrounds();
        break;
    case 3:
        settings.setValue("desktop/background", "inbuilt:shatter");
        emit reloadBackgrounds();
        break;
    case 4:
        settings.setValue("desktop/background", "inbuilt:slice");
        emit reloadBackgrounds();
        break;
    case 5:
        settings.setValue("desktop/background", "inbuilt:nav");
        emit reloadBackgrounds();
        break;
    }
    this->setFocus();
}

void ChooseBackground::on_pushButton_clicked()
{
    delete this;
}

void ChooseBackground::on_pushButton_2_clicked()
{
    ui->lineEdit->setText(QFileDialog::getOpenFileName(this, "Select Background", "", "Images (*.png *.jpg *.jpeg *.bmp *.gif)"));
}
