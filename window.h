#ifndef WINDOW_H
#define WINDOW_H

#include <QObject>
#include <QIcon>

//#include <X11/Xlib.h>

typedef unsigned long Window;

class WmWindow
{

public:
    explicit WmWindow();

    QString title() const;
    void setTitle(QString);

    unsigned long PID() const;
    void setPID(unsigned long);
    Window WID() const;
    void setWID(Window);
    QIcon icon() const;
    void setIcon(QIcon);
    bool attention() const;
    void setAttention(bool attention);
    int desktop() const;
    void setDesktop(int desktop);
    bool isMinimized() const;
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
