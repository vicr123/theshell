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

#include "errordialog.h"
#include "ui_errordialog.h"

#include <the-libs_global.h>

ErrorDialog::ErrorDialog(bool started, int errorCount, QString output, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    QFile backtrace(QDir::homePath() + "/.tsbacktrace");
    if (backtrace.exists()) {
        backtrace.open(QFile::ReadOnly);
        this->backtrace.append(backtrace.readAll());
        backtrace.close();
        backtrace.remove();
    }

    this->backtrace.append("\n\nStandard Output from the main process follows:\n" + output);

    if (this->backtrace == "") {
        ui->debugButton->setVisible(false);
    } else {
        ui->backtrace->setPlainText(this->backtrace);
        ui->backtrace->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    }

    ui->errorIcon->setPixmap(QIcon(":/icons/error.svg").pixmap(128 * theLibsGlobal::instance()->getDPIScaling(), 128 * theLibsGlobal::instance()->getDPIScaling()));
    if (started) {
        if (errorCount >= 3) {
            ui->restartButton->setVisible(false);
            ui->titleLabel->setText(tr("Unfortunately, theShell keeps running into errors."));
            ui->repeatFrame->setVisible(true);
            ui->errorLabel->setVisible(false);
            //ui->errorLabel->setText(tr("theShell keeps running into errors."));
        } else {
            ui->repeatFrame->setVisible(false);
        }
    } else {
        if (errorCount >= 3) {
            ui->restartButton->setVisible(false);
            ui->titleLabel->setText(tr("theShell can't start"));
            ui->repeatFrame->setVisible(true);
            ui->errorLabel->setVisible(false);
        } else {
            ui->repeatFrame->setVisible(false);
            ui->titleLabel->setText(tr("theShell failed to start"));
            ui->errorLabel->setText(tr("theShell wasn't able to start properly."));
            ui->restartButton->setText(tr("Try again"));
        }
    }

    ui->logoutButton->setProperty("type", "destructive");

}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}

void ErrorDialog::on_restartButton_clicked()
{
    emit restart();
}

void ErrorDialog::on_logoutButton_clicked()
{
    emit logout();
}

void ErrorDialog::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void ErrorDialog::on_debugButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void ErrorDialog::on_saveBacktraceButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Backtrace"), QDir::homePath(), tr("Backtrace (*.bt)"));
    if (fileName != "") {
        QFile f(fileName);
        f.open(QFile::WriteOnly);
        f.write(this->backtrace.toUtf8());
        f.close();

        ui->stackedWidget->setCurrentIndex(0);
    }
}

void ErrorDialog::on_resetTSButton_clicked()
{
    if (QMessageBox::warning(this, tr("Reset theShell?"), tr("You're about to reset theShell to its default state. Are you sure you wish to do this?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        QSettings("theSuite", "theShell").clear();

        ui->resetTSButton->setEnabled(false);
        ui->resetTSButton->setText("Settings reset successfully");
    }
}
