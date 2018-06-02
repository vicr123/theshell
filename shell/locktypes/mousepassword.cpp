#include "mousepassword.h"
#include "ui_mousepassword.h"

MousePassword::MousePassword(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MousePassword)
{
    ui->setupUi(this);

    ui->instructionLabel->setText(tr(instructions[0]));
}

MousePassword::~MousePassword()
{
    delete ui;
}

void MousePassword::mousePressEvent(QMouseEvent *event) {
    /*
     * Mouse buttons:
     * LEFT: Q
     * MIDDLE: W
     * RIGHT: E
     * EXTRA1: R
     * EXTRA2: T
     * EXTRA3: Y
     * EXTRA4: U
     * EXTRA5: I
     * EXTRA6: O
     * */

    if (isListening) {
        switch (event->button()) {
            case Qt::LeftButton:
                currentMousePassword += 'q';
                break;
            case Qt::MiddleButton:
                currentMousePassword += 'w';
                break;
            case Qt::RightButton:
                currentMousePassword += 'e';
                break;
            case Qt::ExtraButton1:
                currentMousePassword += 'r';
                break;
            case Qt::ExtraButton2:
                currentMousePassword += 't';
                break;
            case Qt::ExtraButton3:
                currentMousePassword += 'y';
                break;
            case Qt::ExtraButton4:
                currentMousePassword += 'u';
                break;
            case Qt::ExtraButton5:
                currentMousePassword += 'i';
                break;
            case Qt::ExtraButton6:
                currentMousePassword += 'o';
                break;
        }

        if (currentMousePassword.length() > 100) currentMousePassword = currentMousePassword.left(100);
    }

    setMousePasswordLabel();
}

void MousePassword::mouseDoubleClickEvent(QMouseEvent *event) {
    //Treat this as a normal click
    mousePressEvent(event);
}

void MousePassword::mouseReleaseEvent(QMouseEvent *event) {
    /*
     * Mouse buttons:
     * LEFT: A
     * MIDDLE: S
     * RIGHT: D
     * EXTRA1: F
     * EXTRA2: G
     * EXTRA3: H
     * EXTRA4: J
     * EXTRA5: K
     * EXTRA6: L
     * */

    if (isListening) {
        switch (event->button()) {
            case Qt::LeftButton:
                currentMousePassword += 'a';
                break;
            case Qt::MiddleButton:
                currentMousePassword += 's';
                break;
            case Qt::RightButton:
                currentMousePassword += 'd';
                break;
            case Qt::ExtraButton1:
                currentMousePassword += 'f';
                break;
            case Qt::ExtraButton2:
                currentMousePassword += 'g';
                break;
            case Qt::ExtraButton3:
                currentMousePassword += 'h';
                break;
            case Qt::ExtraButton4:
                currentMousePassword += 'j';
                break;
            case Qt::ExtraButton5:
                currentMousePassword += 'k';
                break;
            case Qt::ExtraButton6:
                currentMousePassword += 'l';
                break;
        }

        if (currentMousePassword.length() > 100) currentMousePassword = currentMousePassword.left(100);
    }

    setMousePasswordLabel();
}

void MousePassword::setMousePasswordLabel() {
    QString text = currentMousePassword;
    if (text == "") {
        ui->currentMousePasswordLabel->setText(tr("Go for it!"));
    } else {
        text.replace("q", "L↓ ");
        text.replace("w", "M↓ ");
        text.replace("e", "R↓ ");
        text.replace("r", "1↓ ");
        text.replace("t", "2↓ ");
        text.replace("y", "3↓ ");
        text.replace("u", "4↓ ");
        text.replace("i", "5↓ ");
        text.replace("o", "6↓ ");
        text.replace("a", "L↑ ");
        text.replace("s", "M↑ ");
        text.replace("d", "R↑ ");
        text.replace("f", "1↑ ");
        text.replace("g", "2↑ ");
        text.replace("h", "3↑ ");
        text.replace("j", "4↑ ");
        text.replace("k", "5↑ ");
        text.replace("l", "6↑ ");
        ui->currentMousePasswordLabel->setText(text);
    }
}

void MousePassword::on_resetMousePasswordButton_clicked()
{
    currentMousePassword.clear();
    setMousePasswordLabel();
}

void MousePassword::on_backButton_clicked()
{
    emit exit();
    currentMousePassword.clear();
    tentativeMousePassword.clear();
    setMousePasswordLabel();

    stage = -1;
    on_nextButton_clicked();
}

void MousePassword::on_nextButton_clicked()
{
    stage++;
    ui->instructionLabel->setText(tr(instructions[stage]));
    switch (stage) {
        case 1:
            if (currentMousePassword.length() < 5) {
                ui->instructionLabel->setText(tr("Your Mouse Password needs to contain at least 5 events."));
                currentMousePassword.clear();
                setMousePasswordLabel();
                stage = 0;
            } else {
                tentativeMousePassword = currentMousePassword;
                currentMousePassword.clear();
                setMousePasswordLabel();
            }
            break;
        case 2:
            if (tentativeMousePassword != currentMousePassword) {
                ui->instructionLabel->setText(tr("Unfortunately that wasn't correct. Please try again."));
                currentMousePassword.clear();
                setMousePasswordLabel();
                stage = 1;
            } else {
                tentativeMousePassword.clear();
                ui->nextButton->setText(tr("Set Mouse Password"));
            }
            break;
        case 3: {
            //Check Polkit authorization
            PolkitQt1::Authority::Result r = PolkitQt1::Authority::instance()->checkAuthorizationSync("org.thesuite.theshell.configure-mouse-password", PolkitQt1::UnixProcessSubject(QApplication::applicationPid()), PolkitQt1::Authority::None);

            if (r != PolkitQt1::Authority::Yes) {
                QMessageBox::warning(this, tr("Unauthorized"), tr("Polkit does not allow you to set up a mouse password."), QMessageBox::Ok, QMessageBox::Ok);
                return;
            }

            //Save the mouse password
            QProcess* proc = new QProcess();
            QDir::home().mkdir(".theshell");

            QString executable = "/usr/lib/ts-mousepass-change";
            #ifdef BLUEPRINT
                executable += "b";
            #endif
            proc->start(executable + " --set=" + currentMousePassword + " --passfile=" + QDir::homePath() + "/.theshell/mousepassword");
            proc->waitForFinished();

            if (proc->exitCode() != 0) {
                proc->deleteLater();

                QMessageBox::critical(this->window(), tr("Mouse Password"), tr("Mouse Password couldn't be saved."), QMessageBox::Ok, QMessageBox::Ok);
                return;
            }
            proc->deleteLater();

            exit();
            currentMousePassword.clear();
            setMousePasswordLabel();
            ui->nextButton->setText(tr("Next"));

            stage = 0;
            ui->instructionLabel->setText(tr(instructions[stage]));

            tToast* toast = new tToast();
            toast->setTitle(tr("Mouse Password"));
            toast->setText(tr("Mouse Password was set successfully"));
            connect(toast, SIGNAL(dismissed()), toast, SLOT(deleteLater()));
            toast->show(this->window());
        }
    }
}
