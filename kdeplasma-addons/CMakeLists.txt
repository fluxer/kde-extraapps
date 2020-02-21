project(kdeplasma-addons)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(KDE4Workspace)
set_package_properties(KDE4Workspace PROPERTIES
    DESCRIPTION "KDE base workspace libraries"
    URL "http://fluxer.github.io/katana/"
    PURPOSE "Needed for building several Plasma plugins"
)

macro_optional_find_package(LibAttica 0.1.1)
set_package_properties(LibAttica PROPERTIES
    DESCRIPTION "Attica Library"
    URL "http://websvn.kde.org/trunk/kdesupport/attica/"
    PURPOSE "Needed for building the Open Collaboration Services plasma dataengine and applets"
)

macro_optional_find_package(OpenGL)
set_package_properties(OpenGL PROPERTIES
    DESCRIPTION "API for developing portable, interactive 2D and 3D graphics applications"
    URL "http://mesa3d.sourceforge.net"
    PURPOSE "Used in applications ranging from graphics and modellers to screensavers and video players"
)

macro_optional_find_package(Marble)
set_package_properties(Marble PROPERTIES
    DESCRIPTION "Desktop Globe"
    URL "http://marble.kde.org"
    PURPOSE "Marble is used to provide a desktop globe wallpaper for Plasma"
)

macro_optional_find_package(DBusMenuQt)
set_package_properties(DBusMenuQt PROPERTIES
    DESCRIPTION "Library that provides a Qt implementation of the DBusMenu spec"
    URL "https://github.com/fluxer/libdbusmenu-qt"
    PURPOSE "Needed for Unity support in IconTasks"
)

macro_optional_find_package(Qalculate)
set_package_properties(Qalculate PROPERTIES
    DESCRIPTION "Qalculate Library"
    URL "http://qalculate.sourceforge.net/"
    PURPOSE "Needed for building the Qalculate plasma applet"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(wallpapers)
add_subdirectory(containments)

