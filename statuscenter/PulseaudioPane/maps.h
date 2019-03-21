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
#ifndef MAPS_H
#define MAPS_H

#include <QHash>
#include <pulse/channelmap.h>

QHash<pa_channel_position_t, const char*> channelPositionToString = {
    {PA_CHANNEL_POSITION_INVALID, QT_TRANSLATE_NOOP("maps", "Invalid")},
    {PA_CHANNEL_POSITION_MONO, QT_TRANSLATE_NOOP("maps", "Mono")},
    {PA_CHANNEL_POSITION_FRONT_LEFT, QT_TRANSLATE_NOOP("maps", "Front Left")},
    {PA_CHANNEL_POSITION_FRONT_CENTER, QT_TRANSLATE_NOOP("maps", "Front Center")},
    {PA_CHANNEL_POSITION_FRONT_RIGHT, QT_TRANSLATE_NOOP("maps", "Front Right")},
    {PA_CHANNEL_POSITION_REAR_LEFT, QT_TRANSLATE_NOOP("maps", "Rear Left")},
    {PA_CHANNEL_POSITION_REAR_CENTER, QT_TRANSLATE_NOOP("maps", "Rear Center")},
    {PA_CHANNEL_POSITION_REAR_RIGHT, QT_TRANSLATE_NOOP("maps", "Rear Right")},
    {PA_CHANNEL_POSITION_LFE, QT_TRANSLATE_NOOP("maps", "Subwoofer")},
    {PA_CHANNEL_POSITION_FRONT_LEFT_OF_CENTER, QT_TRANSLATE_NOOP("maps", "Forward Left")},
    {PA_CHANNEL_POSITION_FRONT_RIGHT_OF_CENTER, QT_TRANSLATE_NOOP("maps", "Forward Right")},
    {PA_CHANNEL_POSITION_SIDE_LEFT, QT_TRANSLATE_NOOP("maps", "Left")},
    {PA_CHANNEL_POSITION_SIDE_RIGHT, QT_TRANSLATE_NOOP("maps", "Right")},
    {PA_CHANNEL_POSITION_TOP_CENTER, QT_TRANSLATE_NOOP("maps", "Overhead")},
    {PA_CHANNEL_POSITION_TOP_FRONT_LEFT, QT_TRANSLATE_NOOP("maps", "Overhead Front Left")},
    {PA_CHANNEL_POSITION_TOP_FRONT_CENTER, QT_TRANSLATE_NOOP("maps", "Overhead Front Center")},
    {PA_CHANNEL_POSITION_TOP_FRONT_RIGHT, QT_TRANSLATE_NOOP("maps", "Overhead Front Right")},
    {PA_CHANNEL_POSITION_TOP_REAR_LEFT, QT_TRANSLATE_NOOP("maps", "Overhead Rear Left")},
    {PA_CHANNEL_POSITION_TOP_REAR_CENTER, QT_TRANSLATE_NOOP("maps", "Overhead Rear Center")},
    {PA_CHANNEL_POSITION_TOP_REAR_RIGHT, QT_TRANSLATE_NOOP("maps", "Overhead Rear Right")}
};

#endif // MAPS_H
