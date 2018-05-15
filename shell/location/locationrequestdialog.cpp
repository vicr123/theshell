#include "locationrequestdialog.h"
#include "ui_locationrequestdialog.h"

extern float getDPIScaling();

LocationRequestDialog::LocationRequestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocationRequestDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
    ui->iconLabel->setPixmap(QIcon::fromTheme("gps").pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
    ui->geoclueReason->setVisible(false);
}

LocationRequestDialog::~LocationRequestDialog()
{
    delete ui;
}

void LocationRequestDialog::on_denyButton_clicked()
{
    this->reject();
}

void LocationRequestDialog::setAppName(QString appName) {
    ui->requestMessage->setText(tr("Allow <b>%1</b> to use your physical location?").arg(appName));
    this->appName = appName;
}

void LocationRequestDialog::setIcon(QIcon icon) {
    ui->iconLabel->setPixmap(icon.pixmap(32 * getDPIScaling(), 32 * getDPIScaling()));
}

void LocationRequestDialog::on_allowButton_clicked()
{
    this->accept();
}

void LocationRequestDialog::setReason(QString reason) {
    ui->geoclueReason->setVisible(true);
    //ui->geoclueReason->setText(tr("%1 says \"%2\"").arg(appName, reason));
    ui->geoclueReason->setText(reason);
}
