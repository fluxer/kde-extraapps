/**
 * Copyright (C) 2004 Michael Pyne <mpyne@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "categoryreaderinterface.h"
#include "filerenameroptions.h"

#include <QString>

QString CategoryReaderInterface::value(const CategoryID &category) const
{
    QString value = categoryValue(category.category).trimmed();

    if(category.category == Track)
        value = fixupTrack(value, category.categoryNumber).trimmed();

    if(value.isEmpty()) {
        switch(emptyAction(category)) {
            case TagRenamerOptions::UseReplacementValue:
                value = emptyText(category);
                break;

            case TagRenamerOptions::IgnoreEmptyTag:
                return QString();

            default:
                // No extra action needed.
                break;
        }
    }

    return prefix(category) + value + suffix(category);
}

bool CategoryReaderInterface::isRequired(const CategoryID &category) const
{
    return emptyAction(category) != TagRenamerOptions::IgnoreEmptyTag;
}

bool CategoryReaderInterface::isEmpty(TagType category) const
{
    return categoryValue(category).isEmpty();
}

QString CategoryReaderInterface::fixupTrack(const QString &track, int categoryNum) const
{
    QString str(track);
    CategoryID trackId(Track, categoryNum);

    if(track == "0") {
        if(emptyAction(trackId) == TagRenamerOptions::UseReplacementValue)
            str = emptyText(trackId);
        else
            return QString();
    }

    int minimumWidth = trackWidth(categoryNum);

    if(str.length() < minimumWidth) {
        QString prefix;
        prefix.fill('0', minimumWidth - str.length());
        return prefix + str;
    }

    return str;
}

// vim: set et sw=4 tw=0 sta:
