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
#include "cardwidget.h"
#include "ui_cardwidget.h"

#include <Profile>
#include <Card>

#include <QDebug>

struct CardWidgetPrivate {
    PulseAudioQt::Card* card;
};

CardWidget::CardWidget(PulseAudioQt::Card* card, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CardWidget)
{
    ui->setupUi(this);
    d = new CardWidgetPrivate();

    d->card = card;

    connect(card, &PulseAudioQt::Card::nameChanged, this, [=] {
        ui->nameLabel->setText(card->name());
    });
    connect(card, &PulseAudioQt::Card::activeProfileIndexChanged, this, [=] {
//        for (PulseAudioQt::Profile* prof : card->profiles()) {
//            qDebug() << card->activeProfileIndex() <<  prof->index();
//            if (prof->index() == card->activeProfileIndex()) {
//                ui->profileButton->setText(prof->description());
//            }
//        }
                for (PulseAudioQt::Profile* prof : card->profiles()) {
                    qDebug() << prof->description();
                }
        ui->profileButton->setText(card->profiles().at(card->activeProfileIndex())->description());
    });


    ui->nameLabel->setText(card->name());
//    for (PulseAudioQt::Profile* prof : card->profiles()) {
//        qDebug() << card->activeProfileIndex() <<  prof->index();
//        if (prof->index() == card->activeProfileIndex()) {
//            ui->profileButton->setText(prof->description());
//        }
//    }
    ui->profileButton->setText(card->profiles().at(card->activeProfileIndex())->description());
}

CardWidget::~CardWidget()
{
    delete d;
    delete ui;
}

PulseAudioQt::Card*CardWidget::card()
{
    return d->card;
}

void CardWidget::on_profileButton_clicked()
{

}
