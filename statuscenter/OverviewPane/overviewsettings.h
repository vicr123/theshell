#ifndef OVERVIEWSETTINGS_H
#define OVERVIEWSETTINGS_H

#include <QWidget>
#include <statuscenterpaneobject.h>
#include <QSettings>

namespace Ui {
    class OverviewSettings;
}

class OverviewSettings : public QWidget, public StatusCenterPaneObject
{
        Q_OBJECT

    public:
        explicit OverviewSettings(QWidget *parent = nullptr);
        ~OverviewSettings();

        QWidget* mainWidget();
        QString name();
        StatusPaneTypes type();
        int position();
        void message(QString name, QVariantList args);

    private slots:
        void on_weatherCheckBox_toggled(bool checked);

        void on_celsiusRadio_toggled(bool checked);

        void on_fahrenheitRadio_toggled(bool checked);

    private:
        Ui::OverviewSettings *ui;

        QSettings settings;
};

#endif // OVERVIEWSETTINGS_H
