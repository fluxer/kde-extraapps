# Kate project
project (kde-extraapps)

#search packages used by KDE
set(QT_MIN_VERSION "4.8.2")
find_package(KDE4 4.14.3 REQUIRED)
include (KDE4Defaults)

# shared libs
set (LIBRARY_TYPE SHARED)

# enforce build of tests
set (KDE4_BUILD_TESTS "ON" CACHE "BOOL" "Enable building of tests" FORCE)

# KDE/QT definitions we want
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS -DKTEXTEDITOR_NO_DEPRECATED)

# global include directories
include_directories (${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES})

macro_optional_add_subdirectory (about-distro)
macro_optional_add_subdirectory (ark)
macro_optional_add_subdirectory (bluedevil)
macro_optional_add_subdirectory (dragon)
macro_optional_add_subdirectory (ffmpegthumbs)
macro_optional_add_subdirectory (gwenview)
macro_optional_add_subdirectory (kcalc)
macro_optional_add_subdirectory (kcron)
# macro_optional_add_subdirectory (kdenetwork-filesharing)
macro_optional_add_subdirectory (kfilemetadata)
# macro_optional_add_subdirectory (kget)
macro_optional_add_subdirectory (kgpg)
macro_optional_add_subdirectory (kmix)
macro_optional_add_subdirectory (kolourpaint)
macro_optional_add_subdirectory (kompare)
macro_optional_add_subdirectory (ksnapshot)
macro_optional_add_subdirectory (ksystemlog)
macro_optional_add_subdirectory (kuser)
# macro_optional_add_subdirectory (libktorrent)
macro_optional_add_subdirectory (okular)
macro_optional_add_subdirectory (partitionmanager)
macro_optional_add_subdirectory (print-manager)
