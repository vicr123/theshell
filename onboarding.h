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

private:
    Ui::Onboarding *ui;

    int buttonCurrentLanguage = 0;
    QSettings settings;
};

#endif // ONBOARDING_H
