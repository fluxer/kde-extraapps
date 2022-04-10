project(kdeplasma-addons)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include(GenerateExportHeader)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(KDE4Workspace)
set_package_properties(KDE4Workspace PROPERTIES
    DESCRIPTION "KDE base workspace libraries"
    URL "http://fluxer.github.io/katana/"
    PURPOSE "Needed for building several Plasma plugins"
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

