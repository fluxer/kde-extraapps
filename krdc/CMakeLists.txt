project(krdc)

macro_optional_find_package(LibVNCServer)
macro_log_feature(LIBVNCSERVER_FOUND "libvncserver" "VNC server / client library" "http://libvncserver.sourceforge.net/" FALSE "0.9.8" "Needed to build VNC client support in KRDC")

find_program(FREERDP_EXECUTABLE xfreerdp)
if(FREERDP_EXECUTABLE)
    set(FREERDP_EXECUTABLE_FOUND true)
endif(FREERDP_EXECUTABLE)
macro_log_feature(FREERDP_EXECUTABLE_FOUND "freerdp" "A free Remote Desktop Protocol (RDP) Implementation" "http://www.freerdp.com" FALSE "1.0.2" "Needed for RDP support in KRDC (at runtime)")

# NX support is not ready for KDE 4.2; disabled (uwolfer)
# macro_optional_find_package(LibNXCL)
# macro_log_feature(LIBNXCL_FOUND "libnxcl" "NX X compression client library" "http://svn.berlios.de/svnroot/repos/freenx/trunk/freenx-client/nxcl/" FALSE "1.0" "Needed to build Krdc with NX support")

macro_optional_find_package(TelepathyQt4)
macro_log_feature(TelepathyQt4_FOUND "telepathy-qt" "Telepathy Qt Bindings" "http://telepathy.freedesktop.org" FALSE "0.9" "Needed to build Telepathy Tubes support.")

if(TelepathyQt4_FOUND)
    add_definitions(-DTELEPATHY_SUPPORT)
    include_directories(${TELEPATHY_QT4_INCLUDE_DIR})
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}/core/
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(core)
add_subdirectory(vnc)
add_subdirectory(nx)
add_subdirectory(rdp)
add_subdirectory(test)

if(TelepathyQt4_FOUND)
    add_subdirectory(krdc_approver)
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5010)
add_definitions(-DBUILD_ZEROCONF)

set(krdc_SRCS
    config/hostpreferenceslist.cpp
    config/preferencesdialog.cpp
    floatingtoolbar.cpp
    bookmarkmanager.cpp
    connectiondelegate.cpp
    remotedesktopsmodel.cpp
    systemtrayicon.cpp
    tabbedviewwidget.cpp
    mainwindow.cpp
    main.cpp
)

if(TelepathyQt4_FOUND)
    set(krdc_SRCS ${krdc_SRCS}
        tubesmanager.cpp
    )
endif()

add_executable(krdc ${krdc_SRCS})

target_link_libraries(krdc
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KNOTIFYCONFIG_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    krdccore
)

if(TelepathyQt4_FOUND)
    target_link_libraries(krdc
        ${TELEPATHY_QT4_LIBRARIES}
    )
endif()

target_link_libraries(krdc ${KDE4_KDNSSD_LIBS})

install(TARGETS krdc ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES krdcui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc)
install(PROGRAMS krdc.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES pointcursor.png pointcursormask.png DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc/pics)

if(TelepathyQt4_FOUND)
    configure_file(org.freedesktop.Telepathy.Client.krdc_rfb_handler.service.in
                    ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.krdc_rfb_handler.service)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.krdc_rfb_handler.service
            DESTINATION ${KDE4_DBUS_SERVICES_INSTALL_DIR})

    install(FILES krdc_rfb_handler.client DESTINATION ${KDE4_SHARE_INSTALL_PREFIX}/telepathy/clients/)
endif()

