/*****************************************************************************
*   Copyright (C) 2011, 2012 by Shaun Reich <shaun.reich@kdemail.net>        *
*                                                                            *
*   This program is free software; you can redistribute it and/or            *
*   modify it under the terms of the GNU General Public License as           *
*   published by the Free Software Foundation; either version 2 of           *
*   the License, or (at your option) any later version.                      *
*                                                                            *
*   This program is distributed in the hope that it will be useful,          *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
*   GNU General Public License for more details.                             *
*                                                                            *
*   You should have received a copy of the GNU General Public License        *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
*****************************************************************************/

import QtQuick 1.1
import org.kde.qtextracomponents 0.1

import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
   id: konsoleProfiles

    property int minimumWidth: 200
    property int minimumHeight: 300

    function popupEventSlot(shown) {
        if (shown) {
            view.forceActiveFocus();
        }
    }

    PlasmaCore.DataSource {
        id: profilesSource
        engine: "org.kde.konsoleprofiles"
        onSourceAdded: connectSource(source)
        onSourceRemoved: disconnectSource(source)

        Component.onCompleted: connectedSources = sources
    }

    PlasmaCore.SortFilterModel {
        id: profilesModel
        sortRole: "prettyName"
        sortOrder: "AscendingOrder"
        sourceModel: PlasmaCore.DataModel {
            dataSource: profilesSource
        }
    }

    Component.onCompleted: {
        plasmoid.popupIcon = "utilities-terminal";
        plasmoid.aspectRatioMode = IgnoreAspectRatio;
        plasmoid.popupEvent.connect('popupEvent', popupEventSlot);
    }

   PlasmaCore.Svg {
       id: lineSvg
       imagePath: "widgets/line"
    }

    Row {
        id: headerRow
        anchors { left: parent.left; right: parent.right }

        QIconItem {
            id: appIcon
            icon: QIcon("utilities-terminal")
            width: 32
            height: 32
        }

        PlasmaComponents.Label {
            id: header
            text: i18n("Konsole Profiles")
            horizontalAlignment: Text.AlignHCenter | Text.AlignVCenter
            width: parent.width - appIcon.width * 2
            height: parent.height
        }
    }

    PlasmaCore.SvgItem {
        id: separator

        anchors { left: headerRow.left; right: headerRow.right; top: headerRow.bottom }
        svg: lineSvg
        elementId: "horizontal-line"
        height: lineSvg.elementSize("horizontal-line").height
    }

    Text {
        id: textMetric
        visible: false
        // translated but not used, we just need length/height
        text: i18n("Arbitrary String Which Says Something")
    }

    ListView {
        id: view

        anchors { left: parent.left; right: scrollBar.left; bottom: parent.bottom; top: separator.bottom; topMargin: 5 }

        model: profilesModel
        clip: true
        focus: true
        keyNavigationWraps: true

        delegate: Item {
            id: listdelegate
            height: textMetric.paintedHeight * 2

            anchors {
                left: parent.left
                right: parent.right
            }

            function openProfile() {
                var service = profilesSource.serviceForSource(model["DataEngineSource"])
                var operation = service.operationDescription("open")
                var job = service.startOperationCall(operation)
            }

            PlasmaComponents.Label {
                id: profileText

                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    right: parent.right
                    leftMargin: 10
                    rightMargin: 10
                }

                verticalAlignment: Text.AlignVCenter
                text: model.prettyName
                elide: Text.ElideRight
            }

            MouseArea {
                height: parent.height + 15
                anchors { left: parent.left; right: parent.right;}
                hoverEnabled: true

                onClicked: {
                    openProfile();
                }

                onEntered: {
                    view.currentIndex = index;
                }
            }

            Keys.onPressed: {
                if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return)
                    openProfile();
            }
        }

        highlight: PlasmaComponents.Highlight {
            hover: true
        }

        highlightMoveDuration: 250
        highlightMoveSpeed: 1
    }

    PlasmaComponents.ScrollBar {
        id: scrollBar

        anchors { bottom: parent.bottom; top: separator.top; right: parent.right }

        orientation: Qt.Vertical
        stepSize: view.count / 4
        scrollButtonInterval: view.count / 4

        flickableItem: view
    }
}
