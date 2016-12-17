#include "tutorialwindow.h"
#include "ui_tutorialwindow.h"

TutorialWindow::TutorialWindow(bool doSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TutorialWindow)
{
    ui->setupUi(this);

    //this->setAttribute(Qt::WA_TransparentForMouseEvents);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setFocusPolicy(Qt::NoFocus);
    doMask();

    settings.beginGroup("tutorial");

    unsigned long desktop = 0xFFFFFFFF;
    qDebug() << XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", False),
                     XA_CARDINAL, 32, PropModeReplace, (unsigned char*) &desktop, 1); //Set visible on all desktops

    this->showFullScreen();
    this->doSettings = doSettings;
}

TutorialWindow::~TutorialWindow()
{
    delete ui;
}

void TutorialWindow::doMask() {
    if (maskWidget != NULL) {
        this->setMask(QRegion(maskWidget->x(), maskWidget->y(), maskWidget->sizeHint().width(), maskWidget->sizeHint().height()));
    } else {
        this->setMask(QRegion(-1, -1, 1, 1));
    }
}

void TutorialWindow::resizeEvent(QResizeEvent *event) {
    doMask();
}

void TutorialWindow::showScreen(AvailableScreens screen) {
    switch (screen) {
    case BarLocation:
        if (!settings.value("barLocation", false).toBool() || doSettings) {
            ui->stack->setCurrentWidget(ui->barLocationPage);
            maskWidget = ui->barLocationFrame;
            settings.setValue("barLocation", true);
        }
        break;
    case GatewaySearch:
        if (!settings.value("gatewaySearch", false).toBool() || doSettings) {
            ui->stack->setCurrentWidget(ui->gatewaySearchPage);
            maskWidget = ui->gatewaySearchFrame;
            settings.setValue("gatewaySearch", true);
        }
        break;
    }
    doMask();
}

void TutorialWindow::hideScreen(AvailableScreens screen) {
    bool doSwitch = false;
    switch (screen) {
    case BarLocation:
        if (maskWidget == ui->barLocationFrame) doSwitch = true;
        break;
    case GatewaySearch:
        if (maskWidget == ui->gatewaySearchFrame) doSwitch = true;
        break;

    }

    if (doSwitch) maskWidget = NULL;
    doMask();
}
