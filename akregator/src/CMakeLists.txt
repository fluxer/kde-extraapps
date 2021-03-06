set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

add_subdirectory(about)
add_subdirectory(icons)

include_directories(
  ${PROJECT_SOURCE_DIR}/interfaces
  ${PROJECT_BINARY_DIR}/interfaces
)

########### next target ###############

set(akregator_SRCS main.cpp mainwindow.cpp)

add_executable(akregator ${akregator_SRCS})

target_link_libraries(akregator
    ${KDE4_KPARTS_LIBS}
    akregatorprivate
    akregatorinterfaces
)

install(TARGETS akregator ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(akregatorprivate_LIB_SRCS
    aboutdata.cpp
    trayicon.cpp
    article.cpp
    feed.cpp
    feedlist.cpp
    treenode.cpp
    treenodevisitor.cpp
    utils.cpp
    feediconmanager.cpp
    notificationmanager.cpp
    articlejobs.cpp
    folder.cpp
    kernel.cpp
    subscriptionlistjobs.cpp
    fetchqueue.cpp
    frame.cpp
    framemanager.cpp
    openurlrequest.cpp
    actionmanager.cpp
    actions.cpp
)

add_library(akregatorprivate ${LIBRARY_TYPE} ${akregatorprivate_LIB_SRCS})

target_link_libraries(akregatorprivate
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    akregatorinterfaces
    syndication
)

set_target_properties(akregatorprivate
    PROPERTIES VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)
install(TARGETS akregatorprivate ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)

########### next target ###############

set(akregator_utils_SRCS
    utils/filtercolumnsproxymodel.cpp
)

set(akregatorpart_PART_SRCS
    ${akregator_utils_SRCS}
    deletesubscriptioncommand.cpp
    abstractselectioncontroller.cpp
    articlematcher.cpp
    articlemodel.cpp
    pluginmanager.cpp
    selectioncontroller.cpp
    subscriptionlistmodel.cpp
    searchbar.cpp
    articlelistview.cpp
    actionmanagerimpl.cpp
    createfeedcommand.cpp
    createfoldercommand.cpp
    expireitemscommand.cpp
    articleviewer.cpp
    articleformatter.cpp
    addfeeddialog.cpp
    loadfeedlistcommand.cpp
    editsubscriptioncommand.cpp
    importfeedlistcommand.cpp
    feedpropertiesdialog.cpp
    tabwidget.cpp
    akregator_part.cpp
    mainwidget.cpp
    subscriptionlistview.cpp
    subscriptionlistdelegate.cpp
    dummystorage/storagedummyimpl.cpp
    dummystorage/storagefactorydummyimpl.cpp
    dummystorage/feedstoragedummyimpl.cpp
    speechclient.cpp
    addfeedwidgetbase.ui
    feedpropertieswidgetbase.ui
)

qt4_add_dbus_interfaces(akregatorpart_PART_SRCS ${KDE4_DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSpeech.xml)
qt4_add_dbus_adaptor(akregatorpart_PART_SRCS org.kde.akregator.part.xml akregator_part.h Akregator::Part)

kde4_add_plugin(akregatorpart ${akregatorpart_PART_SRCS})

target_link_libraries(akregatorpart
    akregatorinterfaces
    akregatorprivate
    ${KDE4_KCMUTILS_LIBS}
    ${KDE4_KNOTIFYCONFIG_LIBS}
    ${KDE4_SOLID_LIBS}
    ${KDE4_KPARTS_LIBS}
    syndication
)

install(TARGETS akregatorpart DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(PROGRAMS akregator.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES akregator_part.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES articleviewer.rc pageviewer.rc akregator_part.rc akregator.notifyrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/akregator)
install(FILES akregator_plugin.desktop DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR})
install(FILES akregator_shell.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/akregator)
install(FILES feed.protocol DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES org.kde.akregator.part.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})
