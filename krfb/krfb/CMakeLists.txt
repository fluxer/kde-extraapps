configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config-krfb.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-krfb.h
)

#####################################
# First target: libkrfbprivate - a library
# for linking plugins against.

set(krfbprivate_SRCS
    framebuffer.cpp
    framebufferplugin.cpp
)

add_library(krfbprivate SHARED ${krfbprivate_SRCS})
target_link_libraries(krfbprivate
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${X11_X11_LIB}
    ${LIBVNCSERVER_LIBRARIES}
)

set_target_properties(krfbprivate PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_VERSION}
)

install(
    TARGETS krfbprivate
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
    LIBRARY NAMELINK_SKIP
)

install(
    FILES krfb-framebuffer.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

#####################################
# Second target: krfb - the app
# itself.

set(krfb_SRCS
    connectiondialog.cpp
    events.cpp
    framebuffermanager.cpp
    main.cpp
    mainwindow.cpp
    sockethelpers.cpp
    trayicon.cpp
    rfbservermanager.cpp
    rfbserver.cpp
    rfbclient.cpp
    invitationsrfbserver.cpp
    invitationsrfbclient.cpp
    configtcp.ui
    configsecurity.ui
    connectionwidget.ui
    mainwidget.ui
)

kde4_add_kcfg_files(krfb_SRCS krfbconfig.kcfgc)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/events.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
)

add_executable(krfb ${krfb_SRCS})

target_link_libraries(krfb
    krfbprivate
    vncserver
    ${X11_Xext_LIB}
    ${X11_X11_LIB}
    ${X11_Xdamage_LIB}
    ${X11_XTest_LIB}
    ${QT_QTNETWORK_LIBRARY}
    KDE4::kdnssd
    KDE4::kdeui
    KDE4::kpasswdstore
    ${LIBVNCSERVER_LIBRARIES}
)

install(
    TARGETS krfb
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

########### install files ###############

install(
    PROGRAMS krfb.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

install(
    FILES krfb.notifyrc
    DESTINATION ${KDE4_CONFIG_INSTALL_DIR}/notifications
)
 
