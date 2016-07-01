#include "globalfilter.h"

extern void playSound(QUrl, bool = false);
extern MainWindow* MainWin;
GlobalFilter::GlobalFilter(QApplication *application, QObject *parent) : QObject(parent)
{
   application->installEventFilter(this);

   connect(QApplication::desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(reloadScreens()));
   connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(reloadScreens()));
   connect(QApplication::desktop(), SIGNAL(primaryScreenChanged()), this, SLOT(reloadScreens()));
   connect(MainWin, SIGNAL(reloadBackgrounds()), this, SLOT(reloadBackgrounds()));

   reloadScreens();
}

bool GlobalFilter::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonRelease) {
        QSettings settings;
        if (settings.value("input/touchFeedbackSound", false).toBool()) {
            QSound::play(":/sounds/click.wav");
        }
    }
    return false;
}

void GlobalFilter::reloadScreens() {
    QThread::msleep(500); //Wait for KScreen to apply screen changes
    reloadBackgrounds();
}

void GlobalFilter::reloadBackgrounds() {
    emit removeBackgrounds();
    for (int i = 0; i < QApplication::desktop()->screenCount(); i++) {
        Background* w = new Background(MainWin, QApplication::desktop()->screenGeometry(i));
        w->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint);
        w->setAttribute(Qt::WA_ShowWithoutActivating, true);
        w->show();
        w->setGeometry(QApplication::desktop()->screenGeometry(i));
        w->showFullScreen();
        connect(this, SIGNAL(removeBackgrounds()), w, SLOT(deleteLater()));
    }
}
