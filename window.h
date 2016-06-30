#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>
#include <QIcon>

//#include <X11/Xlib.h>

typedef unsigned long Window;

/*#undef None
#undef Bool
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Expose
#undef Unsorted
#undef Status*/

class WmWindow : public QObject
{
    Q_OBJECT
public:
    explicit WmWindow(QObject *parent = 0);

    QString title();
    void setTitle(QString);

    unsigned long PID();
    void setPID(unsigned long);
    Window WID();
    void setWID(Window);
    QIcon icon();
    void setIcon(QIcon);
    bool attention();
    void setAttention(bool attention);
    int desktop();
    void setDesktop(int desktop);
    bool isMinimized();
    void setMinimized(bool minimized);
signals:

public slots:

private:
    QString winTitle = "";
    int id = 0;
    unsigned long pid = 0;
    Window wid = 0;
    QIcon ic;
    bool attn = false;
    int dk = 0;
    bool min = false;
};

#endif // WINDOW_H
