#include "overviewsettings.h"
#include "ui_overviewsettings.h"

#include <QCheckBox>
#include <QRadioButton>

OverviewSettings::OverviewSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewSettings)
{
    ui->setupUi(this);

    this->settingAttributes.providesLeftPane = false;
    this->settingAttributes.icon = QIcon::fromTheme("preferences-desktop-overview");

    ui->weatherCheckBox->setChecked(settings.value("overview/enableWeather", false).toBool());
    if (settings.value("overview/weatherInCelsius", true).toBool()) {
        ui->celsiusRadio->setChecked(true);
    } else {
        ui->fahrenheitRadio->setChecked(true);
    }
}

OverviewSettings::~OverviewSettings()
{
    delete ui;
}

QWidget* OverviewSettings::mainWidget() {
    return this;
}

QString OverviewSettings::name() {
    return tr("Overview");
}

StatusCenterPaneObject::StatusPaneTypes OverviewSettings::type() {
    return Setting;
}

int OverviewSettings::position() {
    return 0;
}

void OverviewSettings::message(QString name, QVariantList args) {

}

void OverviewSettings::on_weatherCheckBox_toggled(bool checked)
{
    settings.setValue("overview/enableWeather", checked);
}

void OverviewSettings::on_celsiusRadio_toggled(bool checked)
{
    if (checked) {
        settings.setValue("overview/weatherInCelsius", true);
    }
}

void OverviewSettings::on_fahrenheitRadio_toggled(bool checked)
{
    if (checked) {
        settings.setValue("overview/weatherInCelsius", false);
    }
}
