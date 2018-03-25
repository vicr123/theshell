#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QIcon>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include <QStackedWidget>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QSettings>

namespace Ui {
    class ErrorDialog;
}

class ErrorDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit ErrorDialog(int errorCount, QWidget *parent = 0);
        ~ErrorDialog();

    private slots:

        void on_restartButton_clicked();

        void on_logoutButton_clicked();

        void on_backButton_clicked();

        void on_debugButton_clicked();

        void on_saveBacktraceButton_clicked();

        void on_resetTSButton_clicked();

    signals:
        void restart();
        void logout();

    private:
        Ui::ErrorDialog *ui;
        QString backtrace;
};

#endif // ERRORDIALOG_H
