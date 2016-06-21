#include "systrayicons.h"

#define None 0L

extern NativeEventFilter* NativeFilter;

SysTrayIcons::SysTrayIcons(QWidget *parent) : QFrame(parent)
{
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setSpacing(6);
    this->setLayout(layout);

    connect(NativeFilter, SIGNAL(SysTrayEvent(long,long,long,long)), this, SLOT(SysTrayEvent(long,long,long,long)));

    unsigned long selection = 0;
    QString atomName = QString("_NET_SYSTEM_TRAY_S").append(QString::number(XScreenNumberOfScreen(XDefaultScreenOfDisplay(QX11Info::display()))));
    selection = XInternAtom(QX11Info::display(), atomName.toLocal8Bit(), False);
    if (selection == None) {
        QLabel* errorLabel = new QLabel();
        errorLabel->setText("System Tray Unavailable.");
        this->layout()->addWidget(errorLabel);
    } else {
        if (XGetSelectionOwner(QX11Info::display(), selection) == None) {
            int retval = XSetSelectionOwner(QX11Info::display(), selection, this->winId(), CurrentTime);
            if (XGetSelectionOwner(QX11Info::display(), selection) == this->winId()) {
                qDebug() << "Success!";

                /*QTimer* timer = new QTimer();
                timer->setInterval(2000);
                connect(timer, SIGNAL(timeout()), this, SLOT(checkForSysTrayIcons()));
                timer->start();*/
            } else {
                QLabel* errorLabel = new QLabel();
                errorLabel->setText("System Tray Unavailable.");
                this->layout()->addWidget(errorLabel);
            }
        } else { //Systray is already handled.
            QLabel* errorLabel = new QLabel();
            errorLabel->setText("System Tray Unavailable.");
            this->layout()->addWidget(errorLabel);
        }
    }
}

void SysTrayIcons::checkForSysTrayIcons() {
    XEvent* event = NULL;

    int success = XCheckWindowEvent(QX11Info::display(), this->winId(), ClientMessage, event);
    if (success == 1) {
        this->layout()->addWidget(QWidget::createWindowContainer(QWindow::fromWinId(event->xclient.data.l[2])));
    }
}

void SysTrayIcons::SysTrayEvent(long opcode, long data2, long data3, long data4) {
    if (opcode == SYSTEM_TRAY_REQUEST_DOCK) {
        QWindow* window = QWindow::fromWinId(data2);
        window->resize(16, 16);

        QWidget* widget = QWidget::createWindowContainer(window);
        widget->setFixedSize(16, 16);
        this->layout()->addWidget(widget);

        connect(window, &QWindow::destroyed, [=]() {
            widget->deleteLater();
        });
        connect(window, &QWindow::visibleChanged, [=](bool visible) {
           widget->setVisible(visible);
        });
    }
}
