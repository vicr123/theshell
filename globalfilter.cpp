#include "globalfilter.h"

extern void playSound(QUrl, bool = false);
GlobalFilter::GlobalFilter(QApplication *application, QObject *parent) : QObject(parent)
{
   application->installEventFilter(this);
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
