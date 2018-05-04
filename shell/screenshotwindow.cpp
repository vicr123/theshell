/****************************************
 *
 *   theShell - Desktop Environment
 *   Copyright (C) 2018 Victor Tran
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

#include "screenshotwindow.h"
#include "ui_screenshotwindow.h"

extern float getDPIScaling();

screenshotWindow::screenshotWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::screenshotWindow)
{
    ui->setupUi(this);

    ui->discardButton->setProperty("type", "destructive");

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    ui->label->setParent(this);
    this->layout()->removeWidget(ui->label);
    ui->label->raise();
    ui->label->installEventFilter(this);
    band = new QRubberBand(QRubberBand::Rectangle, ui->label);
    bandOrigin = QPoint(0, 0);
    band->setGeometry(QRect(bandOrigin, ui->label->size()));

    regionBand = new QRubberBand(QRubberBand::Rectangle, ui->label);
    QPalette regionBandPal = regionBand->palette();
    regionBandPal.setColor(QPalette::Highlight, QColor(0, 150, 255));
    regionBand->setPalette(regionBandPal);
    regionBand->hide();

    QScreen* currentScreen = NULL;
    for (QScreen* screen : QApplication::screens()) {
        if (screen->geometry().contains(QCursor::pos())) {
            currentScreen = screen;
        }
    }

    if (currentScreen != NULL) {
        originalPixmap = currentScreen->grabWindow(0); //, currentScreen->geometry().x(), currentScreen->geometry().y(), currentScreen->geometry().width(), currentScreen->geometry().height());
        screenshotPixmap = originalPixmap;
        ui->label->setPixmap(screenshotPixmap);
        savePixmap = screenshotPixmap;
        selectedRegion.setCoords(0, 0, originalPixmap.width(), originalPixmap.height());

        QSoundEffect* takeScreenshot = new QSoundEffect();
        takeScreenshot->setSource(QUrl("qrc:/sounds/screenshot.wav"));
        takeScreenshot->play();
        connect(takeScreenshot, SIGNAL(playingChanged()), takeScreenshot, SLOT(deleteLater()));


        this->setGeometry(currentScreen->geometry());
        this->setFixedSize(currentScreen->geometry().size());
    } else {

    }

    Atom DesktopWindowTypeAtom;
    DesktopWindowTypeAtom = XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE_NORMAL", False);
    XChangeProperty(QX11Info::display(), this->winId(), XInternAtom(QX11Info::display(), "_NET_WM_WINDOW_TYPE", False),
                     XA_ATOM, 32, PropModeReplace, (unsigned char*) &DesktopWindowTypeAtom, 1); //Change Window Type

}

screenshotWindow::~screenshotWindow()
{
    delete ui;
}

void screenshotWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Escape:
        case Qt::Key_D:
            ui->discardButton->setDown(true);
            break;
        case Qt::Key_Enter: //We need to capture both Enter and Return since it can vary between keyboards
        case Qt::Key_Return:
        case Qt::Key_C:
            ui->copyButton->setDown(true);
            break;
        case Qt::Key_S:
            ui->saveButton->setDown(true);
            break;
    }
}

void screenshotWindow::keyReleaseEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape:
        case Qt::Key_D:
            ui->discardButton->setDown(false);
            ui->discardButton->click();
            break;
        case Qt::Key_Enter: //We need to capture both Enter and Return since it can vary between keyboards
        case Qt::Key_Return:
        case Qt::Key_C:
            ui->copyButton->setDown(false);
            ui->copyButton->click();
            break;
        case Qt::Key_S:
            ui->saveButton->setDown(false);
            ui->saveButton->click();
            break;
    }
}

void screenshotWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.setPen(QColor(0, 0, 0, 0));
    painter.drawRect(0, 0, this->width(), this->height());
}

void screenshotWindow::show() {
    QDialog::showFullScreen();

    originalGeometry = QRect(0, 0, this->width(), this->height());
    QRectF endGeometry = originalGeometry;
    //qreal scaleFactor = endGeometry.width() - (endGeometry.width() - 50);
    //endGeometry.adjust(50, endGeometry.height() * scaleFactor, -50, -endGeometry.height() * scaleFactor);
    //endGeometry.adjust(75, 75, -75, -75);
    /*qreal newHeight = ((endGeometry.width() - 125 * getDPIScaling()) / endGeometry.width()) * endGeometry.height();
    endGeometry.setHeight(newHeight);
    endGeometry.setWidth(endGeometry.width() - 125 * getDPIScaling());
    endGeometry.moveTo(100 * getDPIScaling() / 2, ((ui->label->height() - newHeight) / 2) - 35 * getDPIScaling());*/

    qreal scaleFactor = (endGeometry.width() - 125 * getDPIScaling()) / endGeometry.width();
    endGeometry.setHeight(endGeometry.height() * scaleFactor);
    endGeometry.setWidth(endGeometry.width() * scaleFactor);
    endGeometry.moveLeft(this->width() / 2 - endGeometry.width() / 2);
    endGeometry.moveTop(ui->descriptionLabel->geometry().y() / 2 - endGeometry.height() / 2);

    tVariantAnimation* anim = new tVariantAnimation();
    anim->setStartValue(QRectF(0, 0, this->width(), this->height()));
    anim->setEndValue(endGeometry);
    anim->setDuration(500);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &tVariantAnimation::valueChanged, [=](QVariant value) {
        ui->label->setGeometry(value.toRect());
        ui->label->setFixedSize(value.toRect().size());
    });
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
    clipboard->setPixmap(savePixmap);

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
    savePixmap.save(&screenshotFile, "PNG");
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

bool screenshotWindow::eventFilter(QObject *object, QEvent *event) {
    if (object == ui->label) {
        qreal scaleFactor = ((originalGeometry.width() - 125 * getDPIScaling()) / originalGeometry.width());
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = (QMouseEvent*) event;
            bandOrigin = mouseEvent->pos();

            currentLine.clear();
            currentLine.append(mouseEvent->pos() / scaleFactor);

            beforeDrawPixmap = screenshotPixmap;
            if (!ui->highlightButton->isChecked()) {
                band->setGeometry(QRect(bandOrigin, mouseEvent->pos()).normalized());
                band->show();

                if (ui->regionSelectButton->isChecked()) {
                    regionBand->hide();
                }
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = (QMouseEvent*) event;
            if (mouseEvent->button() != Qt::RightButton) {
                if (ui->highlightButton->isChecked()) {

                    screenshotPixmap = beforeDrawPixmap;
                    currentLine.append(mouseEvent->pos() / scaleFactor);

                    QPainter p(&screenshotPixmap);
                    //p.setRenderHint(QPainter::Antialiasing);

                    QPen pen = QPen(QColor(255, 255, 0, 100));
                    pen.setWidth(25);
                    pen.setCapStyle(Qt::RoundCap);
                    pen.setJoinStyle(Qt::RoundJoin);
                    p.setPen(pen);
                    p.drawPolyline(currentLine);
                    ui->label->setPixmap(screenshotPixmap);
                } else {
                    band->setGeometry(QRect(bandOrigin, mouseEvent->pos()).normalized());
                }
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = (QMouseEvent*) event;

            QRect region;
            region.setLeft(band->geometry().left() / scaleFactor);
            region.setTop(band->geometry().top() / scaleFactor);
            region.setRight(band->geometry().right() / scaleFactor);
            region.setBottom(band->geometry().bottom() / scaleFactor);
            if (ui->regionSelectButton->isChecked()) {
                selectedRegion = region;
                regionBand->setGeometry(band->geometry());
                band->hide();
                regionBand->show();
            } else if (ui->blankerButton->isChecked()) {
                QPainter p(&screenshotPixmap);
                p.setRenderHint(QPainter::Antialiasing);
                p.setBrush(Qt::black);
                p.drawRect(region);
                band->hide();
            } else if (ui->highlightButton->isChecked()) {
                screenshotPixmap = beforeDrawPixmap;
                currentLine.append(mouseEvent->pos() / scaleFactor);

                QPainter p(&screenshotPixmap);
                p.setRenderHint(QPainter::Antialiasing);

                QPen pen = QPen(QColor(255, 255, 0, 100));
                pen.setWidth(25);
                pen.setCapStyle(Qt::RoundCap);
                pen.setJoinStyle(Qt::RoundJoin);
                p.setPen(pen);
                p.drawPolyline(currentLine);
            }
            savePixmap = screenshotPixmap.copy(selectedRegion);
            ui->label->setPixmap(screenshotPixmap);
        }
    }
    return false;
}

void screenshotWindow::close() {
    QDialog::close();
    this->deleteLater();
}

void screenshotWindow::on_resetButton_clicked()
{
    selectedRegion.setCoords(0, 0, originalPixmap.width(), originalPixmap.height());
    screenshotPixmap = originalPixmap;
    ui->label->setPixmap(screenshotPixmap);
    savePixmap = screenshotPixmap;
    regionBand->hide();
}

void screenshotWindow::on_regionSelectButton_clicked()
{
    ui->descriptionLabel->setText(tr("Select a region using the mouse."));
    ui->label->setCursor(QCursor(Qt::CrossCursor));
}

void screenshotWindow::on_blankerButton_clicked()
{
    ui->descriptionLabel->setText(tr("Redact a region using the mouse."));
    ui->label->setCursor(QCursor(Qt::CrossCursor));
}

void screenshotWindow::on_highlightButton_clicked()
{
    ui->descriptionLabel->setText(tr("Highlight part of the image using the mouse."));
    //ui->label->setCursor(QCursor(Qt::ArrowCursor));

    QImage i(25, 25, QImage::Format_ARGB32);

    {
        QPainter p(&i);
        p.setPen(Qt::transparent);
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(QColor(255, 255, 0, 100));
        p.drawEllipse(0, 0, 25, 25);
    }

    QCursor c(QPixmap::fromImage(i), 12, 12);
    ui->label->setCursor(c);
}
