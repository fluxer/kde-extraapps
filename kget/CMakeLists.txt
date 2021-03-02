project(kget)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

macro_optional_find_package(Sqlite)
set_package_properties(Sqlite PROPERTIES
    DESCRIPTION "SQLite is a Binary-Database"
    PURPOSE "Needed for the SQLite-Backend of the KGet-History and the Kopete-Statistic-Plugin"
)

macro_optional_find_package(QGpgme)
set_package_properties(QGpgme PROPERTIES
    DESCRIPTION "The QGpgME library"
    URL "http://www.kde.org"
    PURPOSE "QGpgME is required to have signature verifying support in KGet"
)

macro_optional_find_package(LibMms)
set_package_properties(LibMms PROPERTIES
    DESCRIPTION "Library to read mms streams"
    URL "http://libmms.sourceforge.net"
    PURPOSE "Needed to build KGet mms support"
)

macro_optional_find_package(KDE4Workspace)
set_package_properties(KDE4Workspace PROPERTIES
    DESCRIPTION "KDE base workspace libraries"
    URL "http://fluxer.github.io/katana/"
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

if(CMAKE_BUILD_TYPE MATCHES Debug)
     add_definitions(-DDEBUG)
endif()

# TODO: utterly broken
set(KGET_TESTING FALSE)

if(KGET_TESTING)
    add_definitions(-DDO_KGET_TEST)
endif()

if(QGPGME_FOUND)
    add_definitions(-DHAVE_QGPGME)
    include_directories(${QGPGME_INCLUDE_DIR})
endif(QGPGME_FOUND)

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
endif(KDE4WORKSPACE_FOUND)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(SQLITE_FOUND)
    add_definitions(-DHAVE_SQLITE)
endif(SQLITE_FOUND)


remove_definitions(-DQT_NO_HTTP)

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
   core/datasourcefactory.cpp
   core/kgetkjobadapter.cpp
   core/kuiserverjobs.cpp
   core/kgetglobaljob.cpp
   core/bitset.cpp
   core/download.cpp
   core/transferhistorystore.cpp
   core/linkimporter.cpp
   dbus/dbustransferwrapper.cpp
   dbus/dbusverifierwrapper.cpp
   core/filemodel.cpp
   core/verifier.cpp
   core/verificationthread.cpp
   core/verificationmodel.cpp
   core/verificationdelegate.cpp
   core/signature.cpp
   core/signaturethread.cpp
   core/keydownloader.cpp
   core/urlchecker.cpp
   core/basedialog.cpp
   core/mostlocalurl.cpp
   core/filedeleter.cpp
)

kde4_add_kcfg_files(kgetcore_SRCS conf/settings.kcfgc)

qt4_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.transfer.xml dbus/dbustransferwrapper.h DBusTransferWrapper)
qt4_add_dbus_adaptor(kgetcore_SRCS dbus/org.kde.kget.verifier.xml dbus/dbusverifierwrapper.h DBusVerifierWrapper)

add_library(kgetcore SHARED ${kgetcore_SRCS})

target_link_libraries(kgetcore ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})

if (KDE4WORKSPACE_FOUND)
    target_link_libraries(kgetcore ${KDE4WORKSPACE_KWORKSPACE_LIBS})
endif (KDE4WORKSPACE_FOUND)

if (SQLITE_FOUND)
  target_link_libraries(kgetcore ${QT_QTSQL_LIBRARY})
endif (SQLITE_FOUND)

if (QGPGME_FOUND)
    target_link_libraries(kgetcore ${QGPGME_LIBRARIES})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
endif(QGPGME_FOUND)

set_target_properties(kgetcore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS kgetcore ${INSTALL_TARGETS_DEFAULT_ARGS})

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
   ui/metalinkcreator/metalinkcreator.cpp
   ui/metalinkcreator/generalwidget.cpp
   ui/metalinkcreator/metalinker.cpp
   ui/metalinkcreator/filedlg.cpp
   ui/metalinkcreator/localemodels.cpp
   ui/metalinkcreator/dragdlg.cpp
   ui/metalinkcreator/urlwidget.cpp
   ui/metalinkcreator/filehandler.cpp
   ui/signaturedlg.cpp
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
   ui/metalinkcreator/introduction.ui
   ui/metalinkcreator/generalwidget.ui
   ui/metalinkcreator/files.ui
   ui/metalinkcreator/filedlg.ui
   ui/metalinkcreator/dragdlg.ui
   ui/metalinkcreator/urlwidget.ui
   ui/metalinkcreator/commondata.ui
   ui/signaturedlg.ui
)

qt4_add_dbus_adaptor(kget_SRCS dbus/org.kde.kget.main.xml dbus/dbuskgetwrapper.h DBusKGetWrapper)

add_executable(kget ${kget_SRCS})

target_link_libraries(kget
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${KDE4_KNOTIFYCONFIG_LIBS}
    kgetcore
)

if (KGET_TESTING)
    target_link_libraries(kget
        ${QT_QTTEST_LIBRARY}
    )
endif()

if (QGPGME_FOUND)
    target_link_libraries(kget ${QGPGME_LIBRARIES})
endif(QGPGME_FOUND)

install(TARGETS kget ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kget.notifyrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kget)