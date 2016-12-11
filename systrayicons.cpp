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

    //Register a new DBus service
    QString service = "org.freedesktop.StatusNotifierHost-" + QString::number(QApplication::applicationPid());
    QDBusConnection::sessionBus().registerService(service);

    //Connect to SNI signals
    QDBusConnection::sessionBus().connect("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher", "StatusNotifierItemRegistered", this, SLOT(SniItemRegistered(QString)));
    QDBusConnection::sessionBus().connect("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher", "StatusNotifierItemUnregistered", this, SLOT(SniItemUnregistered(QString)));

    //Tell SNI about a new system tray host
    QDBusMessage message = QDBusMessage::createMethodCall("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher", "RegisterStatusNotifierHost");
    QVariantList messageArgs;
    messageArgs.append(service);
    message.setArguments(messageArgs);
    QDBusConnection::sessionBus().call(message);

    //Get all current SNI items
    QDBusInterface interface("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher");
    for (QString service : interface.property("RegisteredStatusNotifierItems").toStringList()) {
        SniItemRegistered(service);
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

void SysTrayIcons::SniItemRegistered(QString service) {
    if (!availableSniServices.contains(service)) {
        availableSniServices.append(service);

        SniIcon* icon = new SniIcon(service);
        this->layout()->addWidget(icon);
    }
}

void SysTrayIcons::SniItemUnregistered(QString service) {
    if (availableSniServices.contains(service)) {
        availableSniServices.removeAll(service);
    }
}

SniIcon::SniIcon(QString service, QWidget *parent) : QLabel(parent) {
    this->service = service;
    QStringList pathParts = service.split("/");
    service = pathParts.first();
    pathParts.removeFirst();
    QString path = pathParts.join("/");
    path.insert(0, "/");
    interface = new QDBusInterface(service, path, "org.kde.StatusNotifierItem");
    QDBusConnection::sessionBus().connect(service, path, "org.kde.StatusNotifierItem", "NewTitle", this, SLOT(ReloadIcon()));
    QDBusConnection::sessionBus().connect(service, path, "org.kde.StatusNotifierItem", "NewIcon", this, SLOT(ReloadIcon()));
    QDBusConnection::sessionBus().connect(service, path, "org.kde.StatusNotifierItem", "NewAttentionIcon", this, SLOT(ReloadIcon()));
    QDBusConnection::sessionBus().connect(service, path, "org.kde.StatusNotifierItem", "NewOverlayIcon", this, SLOT(ReloadIcon()));
    QDBusConnection::sessionBus().connect(service, path, "org.kde.StatusNotifierItem", "NewToolTip", this, SLOT(ReloadIcon()));
    QDBusConnection::sessionBus().connect(service, path, "org.kde.StatusNotifierItem", "NewStatus", this, SLOT(ReloadIcon()));

    QDBusConnection::sessionBus().connect("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", "org.kde.StatusNotifierWatcher", "StatusNotifierItemUnregistered", this, SLOT(SniItemUnregistered(QString)));
    ReloadIcon();
}

void SniIcon::SniItemUnregistered(QString service) {
    if (service == this->service) {
        this->deleteLater();
    }
}

void SniIcon::ReloadIcon() {
    if (interface->property("IconName").toString() != "") {
        this->setPixmap(QIcon::fromTheme(interface->property("IconName").toString()).pixmap(24, 24));
    } else {
        //TODO: Load other image data
        QDBusMessage message = QDBusMessage::createMethodCall(interface->service(), interface->path(), "org.freedesktop.DBus.Properties", "Get");
        QList<QVariant> messageArguments;
        messageArguments.append("org.kde.StatusNotifierItem");
        messageArguments.append("IconPixmap");
        message.setArguments(messageArguments);

        QDBusReply<QDBusVariant> reply = QDBusConnection::sessionBus().call(message);
        QDBusVariant pixmapsVar = reply.value();

        QDBusArgument pixmaps = pixmapsVar.variant().value<QDBusArgument>();

        QDBusVariant firstPixmapVar;
        pixmaps >> firstPixmapVar;
        pixmaps.endArray();

        QDBusArgument firstPixmap = firstPixmapVar.variant().value<QDBusArgument>();

        firstPixmap.beginArray();

        int width, height;
        QByteArray data;

        firstPixmap >> width >> height >> data;
        firstPixmap.endArray();

        QImage image(width, height, QImage::Format_ARGB32);

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width * 4; x = x + 4) {
                //char dat = data.at(y * width + x);

                unsigned char a, r, g, b;

                b = data.at(y * width * 4 + x + 3);
                g = data.at(y * width * 4 + x + 2);
                r = data.at(y * width * 4 + x + 1);
                a = data.at(y * width * 4 + x);

                QColor col = QColor(r, g, b, a);

                image.setPixelColor(x / 4, y, col);
            }
        }

        this->setPixmap(QPixmap::fromImage(image.scaledToHeight(24, Qt::SmoothTransformation)));
    }

    this->setToolTip(interface->property("Title").toString());
}

void SniIcon::mouseReleaseEvent(QMouseEvent *event) {
    QPoint pos = this->mapToGlobal(event->pos());
    if (event->button() == Qt::LeftButton) {
        interface->call("Activate", pos.x(), pos.y());
    } else if (event->button() == Qt::RightButton) {
        interface->call("ContextMenu", pos.x(), pos.y());
    } else if (event->button() == Qt::MiddleButton) {
        interface->call("SecondaryActivate", pos.x(), pos.y());
    }
}

void SniIcon::wheelEvent(QWheelEvent *event) {
    if (event->orientation() == Qt::Vertical) {
        interface->call("Scroll", event->delta(), "vertical");
    } else {
        interface->call("Scroll", event->delta(), "horizontal");
    }
}
