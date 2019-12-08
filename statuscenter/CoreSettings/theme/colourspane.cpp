/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
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
#include "colourspane.h"
#include "ui_colourspane.h"

#include <QPainter>
#include <the-libs_global.h>

ColoursPane::ColoursPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColoursPane)
{
    ui->setupUi(this);

    //Build a matrix
    struct ColorScheme {
        QString bgName;
        QString settingName;
        QColor background;
        QList<QColor> accent;
    };

    QList<ColorScheme> colorSchemes = {
        {
            tr("Light"),
            "light",
            QColor(235, 235, 235), {
                QColor(0, 200, 255), //Blue
                QColor(0, 255, 128), //Green
                QColor(255, 100, 0), //Orange
                QColor(255, 0, 255), //Pink
                QColor(0, 200, 150) //Turquoise
            }
        },
        {
            tr("Dark"),
            "dark",
            QColor(40, 40, 40), {
                QColor(0, 50, 150), //Blue
                QColor(0, 85, 0), //Green
                QColor(200, 50, 0), //Orange
                QColor(150, 0, 150), //Pink
                QColor(0, 150, 100) //Turquoise
            }
        },
        {
            tr("Black"),
            "black",
            QColor(0, 0, 0), {
                QColor(0, 50, 150), //Blue
                QColor(0, 85, 0), //Green
                QColor(200, 50, 0), //Orange
                QColor(150, 0, 150), //Pink
                QColor(0, 150, 100) //Turquoise
            }
        },
        {
            tr("Gray"),
            "gray",
            QColor(80, 80, 80), {
                QColor(25, 96, 122), //Blue
                QColor(36, 122, 26), //Green
                QColor(176, 111, 31), //Orange
                QColor(220, 58, 212), //Pink
                QColor(39, 130, 115) //Turquoise
            }
        }
    };

    QString accentNames[] = {
        tr("Blue"),
        tr("Green"),
        tr("Orange"),
        tr("Pink"),
        tr("Turquoise")
    };

    for (int i = 0; i < colorSchemes.count(); i++) {
        ColorScheme col = colorSchemes.at(i);

        for (int j = 0; j < col.accent.count(); j++) {
            QPushButton* b = new QPushButton();
            QPixmap pic = this->getThemePicture(col.background, col.accent.at(j), col.bgName, accentNames[j]);
            b->setIcon(pic);
            b->setIconSize(pic.size());
            b->setFixedSize(pic.size() * 1.1);
            ui->colorsLayout->addWidget(b, i, j);
            b->setVisible(true);

            connect(b, &QPushButton::clicked, this, [=] {
                QSettings settings("theSuite", "ts-qtplatform", parent);
                settings.setValue("color/type", col.settingName);
                settings.setValue("color/accent", j);
                emit reject();
            });
        }
    }

}

ColoursPane::~ColoursPane()
{
    delete ui;
}

void ColoursPane::on_backButton_clicked()
{
    emit reject();
}

QPixmap ColoursPane::getThemePicture(QColor background, QColor foreground, QString backgrondName, QString foregroundName)
{
    QPixmap px(SC_DPI(160), SC_DPI(100));

    QPainter* painter = new QPainter(&px);

    QFont font = this->font();
    font.setPixelSize(SC_DPI(20));
    painter->setFont(font);

    auto drawRect = [=](QColor col, QRect rect, QString text) {
        painter->setPen(Qt::transparent);
        painter->setBrush(col);
        painter->drawRect(rect);

        if (qGray(col.rgb()) > 127) {
            painter->setPen(Qt::black);
        } else {
            painter->setPen(Qt::white);
        }

        QFontMetrics m(font);
        QRect textRect;
        textRect.setHeight(m.height());
        textRect.setWidth(m.horizontalAdvance(text));
        textRect.moveCenter(rect.center());
        painter->drawText(textRect, text);
    };

    drawRect(background, QRect(0, 0, SC_DPI(160), SC_DPI(50)), backgrondName);
    drawRect(foreground, QRect(0, SC_DPI(50), SC_DPI(160), SC_DPI(50)), foregroundName);

    painter->end();
    delete painter;

    return px;
}

