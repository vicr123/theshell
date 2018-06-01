#ifndef MOUSEPASSWORD_H
#define MOUSEPASSWORD_H

#include <QWidget>
#include <QByteArray>
#include <QMouseEvent>
#include <QLabel>
#include <QPushButton>
#include <QRandomGenerator>
#include <QDir>
#include <QFile>
#include <unistd.h>
#include <polkit-qt5-1/PolkitQt1/Authority>
#include <QMessageBox>
#include <ttoast.h>

namespace Ui {
    class MousePassword;
}

class MousePassword : public QWidget
{
        Q_OBJECT

    public:
        explicit MousePassword(QWidget *parent = 0);
        ~MousePassword();

    private slots:
        void on_resetMousePasswordButton_clicked();

        void on_backButton_clicked();

        void setMousePasswordLabel();

        void on_nextButton_clicked();

    signals:
        void exit();

    private:
        Ui::MousePassword *ui;

        void mousePressEvent(QMouseEvent* event);
        void mouseReleaseEvent(QMouseEvent* event);

        QByteArray currentMousePassword, tentativeMousePassword;
        bool isListening = true;
        int stage = 0;

        const char* instructions[4] = {
            QT_TR_NOOP("To get started, use the mouse to input a sequence of button events now."),
            QT_TR_NOOP("Now, confirm the Mouse Password you chose by entering it again."),
            QT_TR_NOOP("Your Mouse Password is ready to be saved."),
            "done"
        };
};

#endif // MOUSEPASSWORD_H
