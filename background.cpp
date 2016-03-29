#include "background.h"
#include "ui_background.h"

Background::Background(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Background)
{
    ui->setupUi(this);

    QSettings settings;
    QString backPath = settings.value("desktop/background", "").toString();
    if (backPath == "") {
        backPath = "/usr/share/icons/theos/backgrounds/triangle/1920x1080.png";
        settings.setValue("desktop/background", backPath);
    }

    ui->label->setPixmap(QPixmap(backPath));
}

Background::~Background()
{
    delete ui;
}
