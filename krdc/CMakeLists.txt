project(krdc)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(GenerateExportHeader)
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

kde4_optional_find_package(LibVNCServer 0.9.8)
set_package_properties(LibVNCServer PROPERTIES
    DESCRIPTION "VNC server / client library"
    URL "http://libvncserver.sourceforge.net/"
    PURPOSE "Needed to build VNC client support in KRDC"
)

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
# add_subdirectory(konsole)
add_subdirectory(vnc)
add_subdirectory(rdp)

if(ENABLE_TESTING)
    add_subdirectory(test)
endif()

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
    KDE4::kfile
    KDE4::kio
    KDE4::knotifyconfig
    KDE4::kcmutils
    krdccore
)

target_link_libraries(krdc KDE4::kdnssd)

install(TARGETS krdc DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES krdcui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc)
install(PROGRAMS krdc.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES pointcursor.png pointcursormask.png DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc/pics)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
