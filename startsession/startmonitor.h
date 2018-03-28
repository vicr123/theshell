#ifndef STARTMONITOR_H
#define STARTMONITOR_H

#include <QObject>
#include <loginsplash.h>
#include <QApplication>

class StartMonitor : public QObject
{
        Q_OBJECT
    public:
        explicit StartMonitor(QObject *parent = nullptr);

        bool started();
    signals:
        void questionResponse(QString response);

    public slots:
        void MarkStarted();
        void MarkNotStarted();
        void ShowSplash();
        void HideSplash();

        void SplashQuestion(QString title, QString msg);
        void SplashPrompt(QString title, QString msg);

    private:
        bool s = false;
        LoginSplash* splash;
};

#endif // STARTMONITOR_H
