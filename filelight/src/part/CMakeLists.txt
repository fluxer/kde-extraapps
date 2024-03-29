#######################################################################
# Copyright 2008-2009  Martin Sandsmark <martin.sandsmark@kde.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License or (at your option) version 3 or any later version
# accepted by the membership of KDE e.V. (or its successor approved
# by the membership of KDE e.V.), which shall act as a proxy
# defined in Section 14 of version 3 of the license.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#######################################################################

set(filelight_PART_SRCS
    radialMap/widget.cpp
    radialMap/builder.cpp
    radialMap/map.cpp
    radialMap/widgetEvents.cpp
    radialMap/labels.cpp
    part.cpp
    scan.cpp
    Config.cpp
    settingsDialog.cpp
    fileTree.cpp
    localLister.cpp
    remoteLister.cpp
    summaryWidget.cpp
    dialog.ui
)

kde4_add_plugin(filelightpart ${filelight_PART_SRCS})

target_link_libraries(filelightpart
    KDE4::kdecore
    KDE4::kparts
    KDE4::kdeui
    KDE4::kio
    KDE4::kfile
    KDE4::solid
)

install(
    TARGETS filelightpart
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
