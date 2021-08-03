project(krdc)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include(GenerateExportHeader)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(LibVNCServer 0.9.8)
set_package_properties(LibVNCServer PROPERTIES
    DESCRIPTION "VNC server / client library"
    URL "http://libvncserver.sourceforge.net/"
    PURPOSE "Needed to build VNC client support in KRDC"
)

# NX support is not ready for KDE 4.2; disabled (uwolfer)
# macro_optional_find_package(LibNXCL)
# macro_log_feature(LIBNXCL_FOUND "libnxcl" "NX X compression client library" "http://svn.berlios.de/svnroot/repos/freenx/trunk/freenx-client/nxcl/" FALSE "1.0" "Needed to build Krdc with NX support")

# v1.0.2 required
find_program(FREERDP_EXECUTABLE xfreerdp)
add_feature_info(xfreerdp
    FREERDP_EXECUTABLE
    "A free Remote Desktop Protocol (RDP) Implementation"
)

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

add_definitions(
    -DKDE_DEFAULT_DEBUG_AREA=5010
    -DBUILD_ZEROCONF
)

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

add_executable(krdc ${krdc_SRCS})

target_link_libraries(krdc
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KNOTIFYCONFIG_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    krdccore
)

target_link_libraries(krdc ${KDE4_KDNSSD_LIBS})

install(TARGETS krdc ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES krdcui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc)
install(PROGRAMS krdc.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES pointcursor.png pointcursormask.png DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc/pics)
