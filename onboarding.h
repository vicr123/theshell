/****************************************
 * 
 *   theShell - Desktop Environment
 *   Copyright (C) 2017 Victor Tran
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

#ifndef ONBOARDING_H
#define ONBOARDING_H

#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QLibraryInfo>
#include <QTranslator>
#include <QTimer>
#include "nativeeventfilter.h"

namespace Ui {
class Onboarding;
}

class Onboarding : public QDialog
{
    Q_OBJECT

public:
    explicit Onboarding(QWidget *parent = 0);
    ~Onboarding();

private slots:
    void on_closeButton_clicked();

    void on_stackedWidget_currentChanged(int arg1);
    
    void on_nextButtonFirstPage_clicked();
    
    void on_nextButton_clicked();

    void on_backButton_clicked();

    void on_beginButton_clicked();

    void on_enabletheWaveButton_clicked();

    void on_disabletheWaveButton_clicked();

    void on_changeLanguageButton_clicked();

    void on_localeList_currentRowChanged(int currentRow);

    void on_enableStatusBarButton_clicked();

    void on_disableStatusBarButton_clicked();

    void on_backToSetupButton_clicked();

    void on_logoutButton_clicked();

    void on_powerOffButton_clicked();

private:
    Ui::Onboarding *ui;

    void reject();

    int buttonCurrentLanguage = 0;
    QSettings settings;
    bool onboardingDone = false;
};

#endif // ONBOARDING_H
