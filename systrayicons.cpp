#include "systrayicons.h"

#define None 0L

extern NativeEventFilter* NativeFilter;

SysTrayIcons::SysTrayIcons(QWidget *parent) : QFrame(parent)
{
    //Prepare a layout for the system tray
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->setSpacing(6);
    this->setLayout(layout);

    //Connect the signal to dock a system tray
    connect(NativeFilter, SIGNAL(SysTrayEvent(long,long,long,long)), this, SLOT(SysTrayEvent(long,long,long,long)));

    //Get the correct manager selection
    unsigned long selection = 0;
    QString atomName = QString("_NET_SYSTEM_TRAY_S").append(QString::number(XScreenNumberOfScreen(XDefaultScreenOfDisplay(QX11Info::display()))));
    selection = XInternAtom(QX11Info::display(), atomName.toLocal8Bit(), False);
    if (selection == None) { //Manager selection wasn't found
        QLabel* errorLabel = new QLabel();
        errorLabel->setText("System Tray Unavailable.");
        this->layout()->addWidget(errorLabel);
    } else {
        if (XGetSelectionOwner(QX11Info::display(), selection) == None) {
            XSetSelectionOwner(QX11Info::display(), selection, this->winId(), CurrentTime);
            if (XGetSelectionOwner(QX11Info::display(), selection) != this->winId()) {
                QLabel* errorLabel = new QLabel();
                errorLabel->setText("System Tray Unavailable.");
                this->layout()->addWidget(errorLabel);
            } else { //System tray available. Send a ClientMessage event to tell everyone that a system tray is available.
                XEvent event;

                event.xclient.type = ClientMessage;
                event.xclient.message_type = XInternAtom(QX11Info::display(), "MANAGER", False);
                event.xclient.format = 32;
                event.xclient.data.l[0] = CurrentTime;
                event.xclient.data.l[1] = selection;
                event.xclient.data.l[2] = this->winId();

                int retval = XSendEvent(QX11Info::display(), DefaultRootWindow(QX11Info::display()), False, StructureNotifyMask, &event);
                qDebug() << retval;
            }
        } else { //Systray is already handled.
            QLabel* errorLabel = new QLabel();
            errorLabel->setText("System Tray Unavailable.");
            this->layout()->addWidget(errorLabel);
        }
    }
}

void SysTrayIcons::SysTrayEvent(long opcode, long data2, long data3, long data4) {
    if (opcode == SYSTEM_TRAY_REQUEST_DOCK) { //Check that the system tray wants to be docked
        //Create a XEmbed window for the system tray and dock it in the layout
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
