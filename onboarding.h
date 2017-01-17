#ifndef ONBOARDING_H
#define ONBOARDING_H

#include <QDialog>
#include <QTextBrowser>
#include <QPushButton>
#include <QSettings>
#include <QLabel>

#define ChangelogOnbording "New in theShell 6.0:\n"\
    "- Default system font can be set in theme settings."\
    "- Battery indicator now shows battery for devices connected over KDE Connect\n"\
    "- Added some KDE Connect actions in the status center\n"\
    "- The stopwatch is now shown on the bar when being used."\
    "- New Charge History in the Status Center. Go to the System Status pane in the Status Center to take a look."\
    "- Add scrollbar on the bar when content runs off the bar\n"\
    "- Added Power Stretch.\n"\
    "- Added icon in bar for wireless reception\n"\
    "- theShell now uses the-libs for integration with other the-applications.\n"\
    "- Dragging down on an item in the bar to open the status center has been added\n"\
    "- New notification animation\n"\
    "- When a timer elapses, other audio is made quiter until the timer is dismissed\n"\
    "- Redshift now works overnight\n"\
    "- Notifications can now send sounds through theShell\n"\
    "- theWave now quitens background music while listening\n"\
    "\n"\
    "New in theShell 5.2: \n"\
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
