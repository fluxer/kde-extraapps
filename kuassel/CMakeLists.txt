# Main CMake file for building Kuassel IRC
#
# See INSTALL for possible CMake options (or read the code, Luke)
#####################################################################

# General setup
#####################################################################

project(KuasselIRC)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

# Version
set(QUASSEL_VERSION_STRING "0.12")

# General conveniences
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake about or own modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

# Include various CMake modules...
include(CMakePushCheckState)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCXXSourceCompiles)
include(CMakeDependentOption)
include(FeatureSummary)

# ... and our own stuff
include(QuasselCompileSettings)

# Setting COMPILE_DEFINITIONS_<CONFIG> is deprecated since CMake 3.0 in favor of generator expressions.
# These have existed since CMake 2.8.10; until we depend on that, we have to explicitly enable the old policy.
if (CMAKE_MAJOR_VERSION GREATER 2)
    cmake_policy(SET CMP0043 OLD)
endif()

# Options and variables that can be set on the command line
#####################################################################

# Handle with care
set(QT_PATH "" CACHE PATH "Path to a Qt4 installation to use instead of the system Qt (e.g. for static builds)")

# For static builds, arbitrary extra libs might need to be linked
# Define a comma-separated list here
# e.g. for pgsql, we need -DLINK_EXTRA=pq;crypt
set(LINK_EXTRA "" CACHE STRING "Semicolon-separated list of libraries to be linked")
if (LINK_EXTRA)
    string(REPLACE "," ";" LINK_EXTRA ${LINK_EXTRA})
    link_libraries(${LINK_EXTRA})
endif()

# Set up Qt
#####################################################################

# Select a Qt installation here, if you don't want to use system Qt
if(QT_PATH)
    # FindQt4 will look for the qmake binary in $PATH, so we just prepend QT_PATH
    set(ENV{PATH} ${QT_PATH}/bin:$ENV{PATH})
endif()

# Find package dependencies
#
# Note that you can forcefully disable optional packages
# using -DCMAKE_DISABLE_FIND_PACKAGE_<PkgName>=TRUE
#####################################################################

add_feature_info("QtDBus module" QT_QTDBUS_FOUND "Needed for supporting D-Bus-based notifications and tray icon, used by most modern desktop environments")
if (QT_QTDBUS_FOUND)
    find_package(dbusmenu-qt QUIET CONFIG)
    set_package_properties(dbusmenu-qt PROPERTIES
        TYPE RECOMMENDED
        URL "https://launchpad.net/libdbusmenu-qt"
        DESCRIPTION "a library implementing the DBusMenu specification"
        PURPOSE     "Required for having a context menu for the D-Bus-based tray icon"
    )
endif()

find_package(IndicateQt QUIET)
set_package_properties(IndicateQt PROPERTIES TYPE OPTIONAL
    URL "https://launchpad.net/libindicate-qt/"
    DESCRIPTION "a library to raise flags on DBus for other components of the desktop to pick up and visualize"
    PURPOSE     "Provides integration into the Ayatana notification system used by e.g. Ubuntu"
)

find_package(QCA2 QUIET)
set_package_properties(QCA2 PROPERTIES TYPE RECOMMENDED
    URL "https://projects.kde.org/projects/kdesupport/qca"
    DESCRIPTION "Qt Cryptographic Architecture"
    PURPOSE     "Required for encryption support"
)

# Non-Qt-based packages

# zlib for compression, however we can always fall back to miniz
find_package(ZLIB QUIET REQUIRED)
set_package_properties(ZLIB PROPERTIES TYPE RECOMMENDED
    URL "http://www.zlib.net"
    DESCRIPTION "a popular compression library"
    PURPOSE     "Use the most common library for protocol compression, instead of the bundled miniz implementation"
)


# Execinfo is needed for generating backtraces
find_package(ExecInfo QUIET)
set_package_properties(ExecInfo PROPERTIES TYPE OPTIONAL
    DESCRIPTION "a library for inspecting backtraces"
    PURPOSE "Used for generating backtraces in case of a crash"
)


# Various checks
#####################################################################

# Check for SSL support in Qt
# As there's no easy way to get Qt's configuration in particular for Qt5, let's just compile
# a small test program checking the defines. This works for both Qt4 and Qt5.
cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_INCLUDES ${QT_INCLUDES})
check_cxx_source_compiles("
    #include \"qglobal.h\"
    #if defined QT_NO_OPENSSL || defined QT_NO_SSL
    #  error \"No SSL support\"
    #endif
    int main() {}"
    HAVE_SSL)
cmake_pop_check_state()

if (HAVE_SSL)
    add_definitions(-DHAVE_SSL)
endif()
add_feature_info("SSL support in Qt" HAVE_SSL "Use secure network connections")

# Various settings
##################

# RPATH needs to be set correctly
# Do this down here, since otherwise KDE wants to handle it itself, and fails
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 1)

if(INDICATEQT_FOUND)
  add_definitions(-DXDG_APPS_INSTALL_DIR=${KDE4_XDG_APPS_INSTALL_DIR})
endif()

check_function_exists(umask HAVE_UMASK)
if(HAVE_UMASK)
    add_definitions(-DHAVE_UMASK)
endif(HAVE_UMASK)

configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h @ONLY)

# These variables will be added to the main targets (CORE, QTCLIENT, MONO)
set(COMMON_DEPS ${RC_WIN32})
set(CORE_DEPS )
set(CLIENT_DEPS )

# Add needed subdirs - the order is important, since src needs some vars set by other dirs
add_subdirectory(data)
add_subdirectory(pics)

# Finally, compile the sources
# We want this after displaying the feature summary to avoid ugly
# CMake backtraces in case a required Qt5 modules misses
#####################################################################

add_subdirectory(src)
