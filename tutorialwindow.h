#ifndef TUTORIALWINDOW_H
#define TUTORIALWINDOW_H

#include <QDialog>
#include <QRegion>
#include <QFrame>
#include <QSettings>
#include <QStackedWidget>
#include <QX11Info>
#include <QDebug>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Expose

namespace Ui {
class TutorialWindow;
}

class TutorialWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TutorialWindow(bool doSettings, QWidget *parent = 0);
    ~TutorialWindow();

    enum AvailableScreens {
        BarLocation,
        GatewaySearch
    };

public slots:
    void showScreen(AvailableScreens screen);
    void hideScreen(AvailableScreens screen);

private:
    Ui::TutorialWindow *ui;
    QWidget* maskWidget = NULL;
    QSettings settings;

    bool doSettings;

    void doMask();

    void resizeEvent(QResizeEvent* event);
};

#endif // TUTORIALWINDOW_H
