#ifndef ONBOARDING_H
#define ONBOARDING_H

#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>
#include <QSettings>
#include <QLabel>

#define ChangelogOnbording "New in theShell 5.2: \n"\
    "- New Onboarding experience. Every time theShell updates, you'll get a changelog.\n" \
    "- theWave now uses theCalculator to process calculations.\n" \
    "- theShell now uses its own platform abstraction module to theme your system. Change the theming in theShell Settings.\n" \
    "- theShell now uses the Contemporary theme to display widgets.\n"\
    "- theShell now displays notifications when it detects a connected device\n" \
    ""

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

private:
    Ui::Onboarding *ui;

    QSettings settings;
};

#endif // ONBOARDING_H
