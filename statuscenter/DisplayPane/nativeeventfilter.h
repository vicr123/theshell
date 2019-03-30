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
#ifndef NATIVEEVENTFILTER_H
#define NATIVEEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include "displaypositionwidget.h"

struct NativeEventFilterPrivate;
class NativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
        Q_OBJECT
    public:
        explicit NativeEventFilter(DisplayPositionWidget *parent = nullptr);
        ~NativeEventFilter();

        bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
    signals:

    public slots:

    private:
        NativeEventFilterPrivate* d;
};

#endif // NATIVEEVENTFILTER_H
