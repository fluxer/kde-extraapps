project(kdeplasma-addons)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(KDE4Workspace)
macro_log_feature(KDE4WORKSPACE_FOUND "kdebase workspace" "KDE base workspace libraries" "http://www.kde.org" FALSE "" "Needed for building several Plasma plugins")

set(LIBATTICA_MIN_VERSION "0.1.1")
macro_optional_find_package(LibAttica)
macro_log_feature(LIBATTICA_FOUND "libattica" "Attica Library" "http://websvn.kde.org/trunk/kdesupport/attica/" FALSE "" "Needed for building the Open Collaboration Services plasma dataengine and applets")

macro_optional_find_package(OpenGL)
macro_log_feature(OPENGL_FOUND "OpenGL" "API for developing portable, interactive 2D and 3D graphics applications" "http://mesa3d.sourceforge.net" FALSE "" "STRONGLY RECOMMENDED: The 3D hardware acceleration available through the OpenGL API is used in applications ranging from graphics and modellers to screensavers and video players.")

macro_optional_find_package(Marble)
macro_log_feature(MARBLE_FOUND "Marble" "Desktop Globe" "http://marble.kde.org" FALSE "" "RECOMMENDED: Marble is used to provide a desktop globe wallpaper for Plasma.")

macro_optional_find_package(DBusMenuQt)
macro_log_feature(DBUSMENUQT_FOUND "DBusMenuQt" "DBusMenu Qt" "https://launchpad.net/libdbusmenu-qt/" FALSE "" "Needed for Unity support in IconTasks.")

include_directories(
    ${KDE4WORKSPACE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(wallpapers)
add_subdirectory(containments)

