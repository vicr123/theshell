/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/

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
        explicit ErrorDialog(bool started, int errorCount, QString output, QWidget *parent = 0);
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
