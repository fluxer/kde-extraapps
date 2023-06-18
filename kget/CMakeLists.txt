project(kget)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

kde4_optional_find_package(LibMms)
set_package_properties(LibMms PROPERTIES
    DESCRIPTION "Library to read mms streams"
    URL "http://libmms.sourceforge.net"
    PURPOSE "Needed to build KGet mms support"
)

kde4_optional_find_package(LibTorrent)
set_package_properties(LibTorrent PROPERTIES
    DESCRIPTION "Feature complete C++ bittorrent implementation focusing on efficiency and scalability"
    URL "https://libtorrent.org/"
    PURPOSE "Needed to build KGet torrent support"
)

kde4_optional_find_package(KDE4Workspace)
set_package_properties(KDE4Workspace PROPERTIES
    DESCRIPTION "KDE base workspace libraries"
    URL "https://osdn.net/projects/kde/"
    PURPOSE "Allows 'shutdown after downloads completed' in KGet"
)

set(kget_adaptor_SRCS
    dbus/dbuskgetwrapper.cpp
)
# set(kget_transfer_adaptor_SRCS
#     core/transferhandler.cpp
#     core/transfertreemodel.cpp
# )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5001) 

# TODO: utterly broken
set(KGET_TESTING FALSE)

set(SUPPORTED_KGET_MIMETYPES "application/x-kgetlist;")

if(KGET_TESTING)
    add_definitions(-DDO_KGET_TEST)
endif()

add_subdirectory(conf)
add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(transfer-plugins)
add_subdirectory(extensions)
add_subdirectory(desktop)
add_subdirectory(plasma)
if(KGET_TESTING)
    add_subdirectory(tests)
endif()

if(KDE4WORKSPACE_FOUND)
    add_definitions(-DHAVE_KWORKSPACE)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}
)

# kgetcore

set(kgetcore_SRCS
   core/job.cpp
   core/jobqueue.cpp
   core/kget.cpp
   core/scheduler.cpp
   core/transfertreemodel.cpp
   core/transfertreeselectionmodel.cpp
   core/transfer.cpp
   core/transfergroup.cpp
   core/transfergrouphandler.cpp
   core/transferhandler.cpp
   core/handler.cpp
   core/transfergroupscheduler.cpp
   core/plugin/plugin.cpp
   core/plugin/transferfactory.cpp
   core/transferdatasource.cpp
   core/kgetkjobadapter.cpp
   core/kuiserverjobs.cpp
   core/kgetglobaljob.cpp
   core/bitset.cpp
   core/download.cpp
   core/transferhistorystore.cpp
   core/transferhistorystore_xml.cpp
   core/transferhistorystore_json.cpp
   core/linkimporter.cpp
   dbus/dbustransferwrapper.cpp
   dbus/dbusverifierwrapper.cpp
   core/filemodel.cpp
   core/verifier.cpp
   core/verificationthread.cpp
   core/verificationmodel.cpp
   core/verificationdelegate.cpp
   core/urlchecker.cpp
   core/basedialog.cpp
   core/mostlocalurl.cpp
   core/filedeleter.cpp
)

kde4_add_kcfg_files(kgetcore_SRCS conf/settings.kcfgc)

qt4_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.transfer.xml dbus/dbustransferwrapper.h DBusTransferWrapper)
qt4_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.verifier.xml dbus/dbusverifierwrapper.h DBusVerifierWrapper)

add_library(kgetcore SHARED ${kgetcore_SRCS})

target_link_libraries(kgetcore
    KDE4::kio
    KDE4::solid
)

if (KDE4WORKSPACE_FOUND)
    target_link_libraries(kgetcore KDE4Workspace::kworkspace)
endif (KDE4WORKSPACE_FOUND)

set_target_properties(kgetcore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS kgetcore
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

# kget

set(kget_SRCS ${kget_adaptor_SRCS} ${kget_transfer_adaptor_SRCS}
   conf/autopastemodel.cpp
   conf/integrationpreferences.cpp
   conf/dlgwebinterface.cpp
   conf/preferencesdialog.cpp
   conf/transfersgrouptree.cpp
   conf/transfersgroupwidget.cpp
   conf/pluginselector.cpp
   conf/verificationpreferences.cpp
   ui/droptarget.cpp
   ui/transfersview.cpp
   ui/transfersviewdelegate.cpp
   ui/transferdetails.cpp
   ui/viewscontainer.cpp
   ui/newtransferdialog.cpp
   ui/groupsettingsdialog.cpp
   ui/transfersettingsdialog.cpp
   ui/contextmenu.cpp
   ui/tray.cpp
   ui/history/rangetreewidget.cpp
   ui/history/transferhistory.cpp
   ui/history/transferhistoryitemdelegate.cpp
   ui/history/transferhistorycategorizeddelegate.cpp
   ui/history/transferhistorycategorizedview.cpp
   ui/linkview/kget_linkview.cpp
   ui/linkview/kget_sortfilterproxymodel.cpp
   ui/mirror/mirrorsettings.cpp
   ui/mirror/mirrormodel.cpp
   ui/renamefile.cpp
   ui/verificationdialog.cpp
   mainwindow.cpp
   main.cpp

   extensions/webinterface/httpserver.cpp

   conf/dlgadvanced.ui
   conf/dlgappearance.ui
   conf/dlggroups.ui
   conf/dlgintegration.ui
   conf/dlgwebinterface.ui
   conf/dlgnetwork.ui
   conf/verificationpreferences.ui
   ui/transferdetailsfrm.ui
   ui/newtransferwidget.ui
   ui/history/transferhistory.ui
   ui/groupsettingsdialog.ui
   ui/transfersettingsdialog.ui
   ui/linkview/importlinkdialog.ui
   ui/mirror/mirrorsettings.ui
   ui/mirror/mirroradddlg.ui
   ui/renamefile.ui
   ui/verificationdialog.ui
   ui/verificationadddlg.ui
)

if(KGET_TESTING)
    set(kget_SRCS
        ${kget_SRCS}
        tests/testkget.cpp
    )
endif()

qt4_add_dbus_adaptor(kget_SRCS dbus/org.kde.kget.main.xml dbus/dbuskgetwrapper.h DBusKGetWrapper)

add_executable(kget ${kget_SRCS})

target_link_libraries(kget
    KDE4::kdeui
    KDE4::kio
    KDE4::kcmutils
    KDE4::knotifyconfig
    KDE4::kpasswdstore
    kgetcore
)

if (KGET_TESTING)
    target_link_libraries(kget
        ${QT_QTTEST_LIBRARY}
    )
endif()

install(
    TARGETS kget
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

install(
    FILES kget.notifyrc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kget
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
