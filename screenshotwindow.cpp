#include "screenshotwindow.h"
#include "ui_screenshotwindow.h"

screenshotWindow::screenshotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::screenshotWindow)
{
    ui->setupUi(this);

    ui->discardButton->setProperty("type", "destructive");

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->label->setParent(this);
    ui->label->raise();

    QScreen* currentScreen = NULL;
    for (QScreen* screen : QApplication::screens()) {
        if (screen->geometry().contains(QCursor::pos())) {
            currentScreen = screen;
        }
    }

    if (currentScreen != NULL) {
        screenshotPixmap = currentScreen->grabWindow(0, currentScreen->geometry().x(), currentScreen->geometry().y(), currentScreen->geometry().width(), currentScreen->geometry().height());
        ui->label->setPixmap(screenshotPixmap);

        QSoundEffect* takeScreenshot = new QSoundEffect();
        takeScreenshot->setSource(QUrl("qrc:/sounds/screenshot.wav"));
        takeScreenshot->play();
        connect(takeScreenshot, SIGNAL(playingChanged()), takeScreenshot, SLOT(deleteLater()));


        this->setGeometry(currentScreen->geometry());
        this->setFixedSize(currentScreen->geometry().size());
    } else {

    }
}

screenshotWindow::~screenshotWindow()
{
    delete ui;
}

void screenshotWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.setPen(QColor(0, 0, 0, 0));
    painter.drawRect(0, 0, this->width(), this->height());
}

void screenshotWindow::show() {
    QDialog::show();

    ui->label->setGeometry(0, 0, this->width(), this->height());

    QRectF endGeometry = ui->label->geometry();
    //qreal scaleFactor = endGeometry.width() - (endGeometry.width() - 50);
    //endGeometry.adjust(50, endGeometry.height() * scaleFactor, -50, -endGeometry.height() * scaleFactor);
    //endGeometry.adjust(75, 75, -75, -75);
    qreal newHeight = ((endGeometry.width() - 100) / endGeometry.width()) * endGeometry.height();
    endGeometry.setHeight(newHeight);
    endGeometry.setWidth(endGeometry.width() - 100);
    endGeometry.moveTo(100 / 2, ((ui->label->height() - newHeight) / 2) - 25);

    QPropertyAnimation* anim = new QPropertyAnimation(ui->label, "geometry");
    anim->setStartValue(ui->label->geometry());
    anim->setEndValue(endGeometry);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, SIGNAL(finished()), anim, SLOT(deleteLater()));
    anim->start();
}

void screenshotWindow::on_discardButton_clicked()
{
    QRect newGeometry = ui->label->geometry();
    newGeometry.moveTop(this->height() / 2);

    QParallelAnimationGroup* animGroup = new QParallelAnimationGroup();

    QPropertyAnimation* anim = new QPropertyAnimation(ui->label, "geometry");
    anim->setStartValue(ui->label->geometry());
    anim->setEndValue(newGeometry);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InQuint);
    animGroup->addAnimation(anim);

    QPropertyAnimation* closeAnim = new QPropertyAnimation(this, "windowOpacity");
    closeAnim->setStartValue(1);
    closeAnim->setEndValue(0);
    closeAnim->setDuration(500);
    closeAnim->setEasingCurve(QEasingCurve::InQuint);
    animGroup->addAnimation(closeAnim);

    connect(animGroup, SIGNAL(finished()), animGroup, SLOT(deleteLater()));
    connect(animGroup, SIGNAL(finished()), this, SLOT(close()));
    animGroup->start();
}

void screenshotWindow::on_copyButton_clicked()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setPixmap(screenshotPixmap);

    QRect newGeometry = ui->label->geometry();
    newGeometry.moveTop(-this->height() / 2);

    QParallelAnimationGroup* animGroup = new QParallelAnimationGroup();

    QPropertyAnimation* anim = new QPropertyAnimation(ui->label, "geometry");
    anim->setStartValue(ui->label->geometry());
    anim->setEndValue(newGeometry);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InQuint);
    animGroup->addAnimation(anim);

    QPropertyAnimation* closeAnim = new QPropertyAnimation(this, "windowOpacity");
    closeAnim->setStartValue(1);
    closeAnim->setEndValue(0);
    closeAnim->setDuration(500);
    closeAnim->setEasingCurve(QEasingCurve::InQuint);
    animGroup->addAnimation(closeAnim);

    connect(animGroup, SIGNAL(finished()), animGroup, SLOT(deleteLater()));
    connect(animGroup, SIGNAL(finished()), this, SLOT(close()));
    animGroup->start();
}

void screenshotWindow::on_saveButton_clicked()
{
    QFile screenshotFile(QDir::homePath() + "/screenshot" + QDateTime::currentDateTime().toString("hh-mm-ss-yyyy-MM-dd") + ".png");
    screenshotPixmap.save(&screenshotFile, "PNG");

    QRect newGeometry = ui->label->geometry();
    newGeometry.moveTop(-this->height() / 2);

    QParallelAnimationGroup* animGroup = new QParallelAnimationGroup();

    QPropertyAnimation* anim = new QPropertyAnimation(ui->label, "geometry");
    anim->setStartValue(ui->label->geometry());
    anim->setEndValue(newGeometry);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::InQuint);
    animGroup->addAnimation(anim);

    QPropertyAnimation* closeAnim = new QPropertyAnimation(this, "windowOpacity");
    closeAnim->setStartValue(1);
    closeAnim->setEndValue(0);
    closeAnim->setDuration(500);
    closeAnim->setEasingCurve(QEasingCurve::InQuint);
    animGroup->addAnimation(closeAnim);

    connect(animGroup, SIGNAL(finished()), animGroup, SLOT(deleteLater()));
    connect(animGroup, SIGNAL(finished()), this, SLOT(close()));
    animGroup->start();
}

void screenshotWindow::close() {
    QDialog::close();
    this->deleteLater();
}
