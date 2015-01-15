/*
 *   Copyright 2012 Alex Merry <alex.merry@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    id: root

    property Mpris2 source
    property int contentLeftOffset: 3
    property bool showArtist: true
    property bool showAlbum: true

    implicitHeight: childrenRect.height
    implicitWidth: contentLeftOffset + Math.max(Math.max(titleLabel.implicitWidth, artistLabel.implicitWidth), albumLabel.implicitWidth)
    height: childrenRect.height

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', function(){
            showArtist = plasmoid.readConfig("displayArtist");
            showAlbum = plasmoid.readConfig("displayAlbum");
        });
    }

    function getLabelSize(weight) 
    {
      return (parent.height + (parent.width /6)) / weight;
    }
    Label {
        id: titleLabel
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: contentLeftOffset
            topMargin: 10
            right: parent.right
        }
	font.weight: Font.Bold
        font.pointSize: getLabelSize(16)
        text: source.title
    }
    Label {
        id: artistLabel
        font.pointSize: getLabelSize(18)
        anchors {
            top: titleLabel.bottom
            left: parent.left
            leftMargin: contentLeftOffset
            right: parent.right
        }        
        visible: showArtist && text != ''
        text: source.artist
    }
    Label {
        id: albumLabel
        
        font.pointSize: getLabelSize(18)
        anchors {
            top: artistLabel.visible ? artistLabel.bottom : titleLabel.bottom
            left: parent.left
            leftMargin: contentLeftOffset
            right: parent.right
        }
        visible: showAlbum && text != ''
        text: source.album
    }
}

// vi:sts=4:sw=4:et
