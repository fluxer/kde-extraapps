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

set(partitionmanagerprivate_SRCS
    backend/corebackend.cpp
    backend/corebackenddevice.cpp
    backend/corebackendmanager.cpp
    backend/corebackendpartition.cpp
    backend/corebackendpartitiontable.cpp
    core/copysource.cpp
    core/copysourcedevice.cpp
    core/copysourcefile.cpp
    core/copysourceshred.cpp
    core/copytarget.cpp
    core/copytargetdevice.cpp
    core/copytargetfile.cpp
    core/device.cpp
    core/devicescanner.cpp
    core/mountentry.cpp
    core/operationrunner.cpp
    core/operationstack.cpp
    core/partitionalignment.cpp
    core/partition.cpp
    core/partitionnode.cpp
    core/partitionrole.cpp
    core/partitiontable.cpp
    util/capacity.cpp
    util/externalcommand.cpp
    util/globallog.cpp
    util/helpers.cpp
    util/htmlreport.cpp
    util/report.cpp
    ops/backupoperation.cpp
    ops/checkoperation.cpp
    ops/copyoperation.cpp
    ops/createfilesystemoperation.cpp
    ops/createpartitiontableoperation.cpp
    ops/deleteoperation.cpp
    ops/newoperation.cpp
    ops/operation.cpp
    ops/resizeoperation.cpp
    ops/restoreoperation.cpp
    ops/setfilesystemlabeloperation.cpp
    ops/setpartflagsoperation.cpp
    jobs/backupfilesystemjob.cpp
    jobs/checkfilesystemjob.cpp
    jobs/copyfilesystemjob.cpp
    jobs/createfilesystemjob.cpp
    jobs/createpartitionjob.cpp
    jobs/createpartitiontablejob.cpp
    jobs/deletefilesystemjob.cpp
    jobs/deletepartitionjob.cpp
    jobs/job.cpp
    jobs/movefilesystemjob.cpp
    jobs/resizefilesystemjob.cpp
    jobs/restorefilesystemjob.cpp
    jobs/setfilesystemlabeljob.cpp
    jobs/setpartflagsjob.cpp
    jobs/setpartgeometryjob.cpp
    jobs/shredfilesystemjob.cpp
    fs/btrfs.cpp
    fs/exfat.cpp
    fs/ext2.cpp
    fs/ext3.cpp
    fs/ext4.cpp
    fs/extended.cpp
    fs/fat16.cpp
    fs/fat32.cpp
    fs/filesystem.cpp
    fs/filesystemfactory.cpp
    fs/hfs.cpp
    fs/hfsplus.cpp
    fs/hpfs.cpp
    fs/jfs.cpp
    fs/linuxswap.cpp
    fs/luks.cpp
    fs/lvm2_pv.cpp
    fs/nilfs2.cpp
    fs/ntfs.cpp
    fs/ocfs2.cpp
    fs/reiser4.cpp
    fs/reiserfs.cpp
    fs/ufs.cpp
    fs/unformatted.cpp
    fs/unknown.cpp
    fs/xfs.cpp
    fs/zfs.cpp
    gui/applyprogressdetailswidget.cpp
    gui/applyprogressdialog.cpp
    gui/applyprogressdialogwidget.cpp
    gui/createpartitiontabledialog.cpp
    gui/createpartitiontablewidget.cpp
    gui/decryptluksdialog.cpp
    gui/decryptluksdialogwidget.cpp
    gui/devicepropsdialog.cpp
    gui/devicepropswidget.cpp
    gui/editmountoptionsdialog.cpp
    gui/editmountoptionsdialogwidget.cpp
    gui/editmountpointdialog.cpp
    gui/editmountpointdialogwidget.cpp
    gui/filesystemsupportdialog.cpp
    gui/filesystemsupportdialogwidget.cpp
    gui/formattedspinbox.cpp
    gui/infopane.cpp
    gui/insertdialog.cpp
    gui/listdevices.cpp
    gui/listoperations.cpp
    gui/mainwindow.cpp
    gui/newdialog.cpp
    gui/partitionmanagerwidget.cpp
    gui/partpropsdialog.cpp
    gui/partresizerwidget.cpp
    gui/parttablewidget.cpp
    gui/partwidgetbase.cpp
    gui/partwidget.cpp
    gui/resizedialog.cpp
    gui/scanprogressdialog.cpp
    gui/sizedetailswidget.cpp
    gui/sizedialogbase.cpp
    gui/sizedialogwidget.cpp
    gui/treelog.cpp
    config/advancedpagewidget.cpp
    config/configureoptionsdialog.cpp
    config/filesystemcolorspagewidget.cpp
    config/generalpagewidget.cpp
)

if(LIBATASMART_FOUND)
    include_directories(${LIBATASMART_INCLUDES})
    add_definitions(-DHAVE_LIBATASMART)

    set(partitionmanagerprivate_SRCS
        ${partitionmanagerprivate_SRCS}
        core/smartattribute.cpp
        core/smartstatus.cpp
        gui/smartdialog.cpp
        gui/smartdialogwidget.cpp
    )
endif()

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

target_link_libraries(partitionmanagerprivate
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_SOLID_LIBS}
    ${UUID_LIBRARIES}
    ${BLKID_LIBRARIES}
)

if(LIBATASMART_FOUND)
    target_link_libraries(partitionmanagerprivate
        ${LIBATASMART_LIBRARIES}
    )
endif()

install(TARGETS partitionmanagerprivate ${INSTALL_TARGETS_DEFAULT_ARGS})

############################################

add_executable(partitionmanager main.cpp)

target_link_libraries(partitionmanager partitionmanagerprivate ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS partitionmanager ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES gui/partitionmanagerui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/partitionmanager)
install(PROGRAMS partitionmanager.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

############################################

add_subdirectory(plugins)
