# Copyright (C) 2008,2012 by Volker Lanz <vl@fidra.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

############################################

file(GLOB partitionmanagerprivate_SRCS
    backend/*.cpp
    core/*.cpp
    util/*.cpp
    ops/*.cpp
    jobs/*.cpp
    fs/*.cpp
    gui/*.cpp
    config/*.cpp
)

# avoid putting the generated files in target specific
# directory which is the case when automated
set(CMAKE_AUTOUIC OFF)

qt4_wrap_ui(partitionmanagerprivate_SRCS
    config/configurepageadvanced.ui
    config/configurepagefilesystemcolors.ui
    config/configurepagegeneral.ui
    gui/smartdialogwidgetbase.ui
    gui/decryptluksdialogwidgetbase.ui
    gui/devicepropswidgetbase.ui
    gui/mainwindowbase.ui
    gui/treelogbase.ui
    gui/partitionmanagerwidgetbase.ui
    gui/partpropswidgetbase.ui
    gui/listdevicesbase.ui
    gui/applyprogressdetailswidgetbase.ui
    gui/filesystemsupportdialogwidgetbase.ui
    gui/sizedetailswidgetbase.ui
    gui/createpartitiontablewidgetbase.ui
    gui/listoperationsbase.ui
    gui/sizedialogwidgetbase.ui
    gui/editmountpointdialogwidgetbase.ui
    gui/editmountoptionsdialogwidgetbase.ui
    gui/applyprogressdialogwidgetbase.ui
)

kde4_add_kcfg_files(partitionmanagerprivate_SRCS config.kcfgc)

add_library(partitionmanagerprivate SHARED ${partitionmanagerprivate_SRCS})

target_link_libraries(partitionmanagerprivate libfatlabel ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KIO_LIBS} ${UUID_LIBRARIES} ${BLKID_LIBRARIES} ${LIBATASMART_LIBRARIES} ${KDE4_KDEUI_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS partitionmanagerprivate ${INSTALL_TARGETS_DEFAULT_ARGS})

############################################

add_executable(partitionmanager main.cpp)

target_link_libraries(partitionmanager partitionmanagerprivate ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS partitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/partitionmanager.appdata.xml DESTINATION share/appdata)
install(FILES gui/partitionmanagerui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/partitionmanager)
install(PROGRAMS partitionmanager.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

############################################

add_subdirectory(plugins)
