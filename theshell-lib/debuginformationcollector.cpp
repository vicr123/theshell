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
#include "debuginformationcollector.h"

#include <QWidget>
#include <QBuffer>
#include <tpromise.h>

struct DebugInformationCollectorPrivate {
    DebugInformationCollector* instance = nullptr;
    QObject* rootObject = new QObject();

    void iterateTree(QObject* object, int startLevel, std::function<void(int, QObject*)> callback) {
        callback(startLevel, object);
        for (QObject* child : object->children()) {
            iterateTree(child, startLevel + 1, callback);
        }
    }
};

DebugInformationCollectorPrivate* DebugInformationCollector::d = new DebugInformationCollectorPrivate;

DebugInformationCollector::DebugInformationCollector(QObject *parent) : QObject(parent)
{

}

void DebugInformationCollector::makeInstance() {

}

QObject* DebugInformationCollector::rootParent() {
    return d->rootObject;
}

void DebugInformationCollector::getDebugInformation(QIODevice* data) {
    QSharedPointer<QList<QPointer<QObject>>> objects(new QList<QPointer<QObject>>());

    auto exportData = [=](int level, QObject* object) {
        QString line;
        line.fill(' ', level);
        line.append(object->metaObject()->className());
        line.append(QString(" 0x%1").arg(reinterpret_cast<quintptr>(object), QT_POINTER_SIZE * 2, 16, QChar('0')));
        line.append("\n");
        data->write(line.toUtf8());

        if (!objects->contains(object)) {
            objects->append(object);
        }
    };

    data->open(QBuffer::WriteOnly);
    data->write(QString("Tree of QObjects being tracked:\n").toUtf8());
    d->iterateTree(d->rootObject, 0, exportData);

    data->write(QString("\n").toUtf8());
    data->write(QString("Tree of QWidgets being tracked:\n").toUtf8());
    for (QWidget* widget : QApplication::allWidgets()) {
        d->iterateTree(widget, 0, exportData);
    }
    QList<QPair<int, QByteArray>> occurences;

    data->write(QString("\n").toUtf8());
    data->write(QString("Information about each QObject:\n").toUtf8());
    for (QPointer<QObject> object : *objects.data()) {
        if (object.isNull()) continue;
        QByteArray className(object->metaObject()->className());

        QString firstLine;
        firstLine.append(className);
        firstLine.append(QString(" 0x%1").arg(reinterpret_cast<quintptr>(object.data()), QT_POINTER_SIZE * 2, 16, QChar('0')));
        firstLine.append(QString(" \"%1\"").arg(object->objectName()));
        firstLine.append("\n");
        data->write(firstLine.toUtf8());

        //int padding = QString(object->metaObject()->className()).length();
        QList<QByteArray> propertyNames;
        for (int i = 0; i < object->metaObject()->propertyCount(); i++) {
            propertyNames.append(QByteArray(object->metaObject()->property(i).name()));
        }
        propertyNames.append(object->dynamicPropertyNames());
        for (QByteArray propertyName : propertyNames) {
            QString line;
            line.fill(' ', 4);
            line.append(QString("%1 = %2").arg(QString(propertyName)).arg(object->property(propertyName.data()).toString()));
            line.append("\n");
            data->write(line.toUtf8());
        }

        bool add = true;
        for (auto i = occurences.begin(); i != occurences.end(); i++) {
            if (i->second == className) {
                i->first++;
                add = false;
                break;
            }
        }

        if (add) {
            occurences.append(QPair<int, QByteArray>(1, className));
        }
    }

    data->write(QString("\n").toUtf8());
    data->write(QString("Number of occurences of each QObject:\n").toUtf8());
    std::sort(occurences.begin(), occurences.end(), [=](const QPair<int, QByteArray>& first, const QPair<int, QByteArray>& second) {
        return first.first > second.first;
    });

    for (QPair<int, QByteArray> object : occurences) {
        data->write(QString("%1 %2\n").arg(object.first).arg(QString(object.second)).toUtf8());
    }
}
