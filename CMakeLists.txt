project(kde-extraapps)

find_package(KDE4 4.21.0 REQUIRED)
include(KDE4Defaults)
include(GenerateExportHeader)

set(LIBRARY_TYPE SHARED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

# global include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
)

macro_optional_add_subdirectory(about-distro)
macro_optional_add_subdirectory(ark)
macro_optional_add_subdirectory(filelight)
macro_optional_add_subdirectory(gwenview)
macro_optional_add_subdirectory(jovie)
macro_optional_add_subdirectory(kcalc)
macro_optional_add_subdirectory(kcolorchooser)
macro_optional_add_subdirectory(kcron)
macro_optional_add_subdirectory(kdenetwork-filesharing)
macro_optional_add_subdirectory(kdeplasma-addons)
macro_optional_add_subdirectory(kemu)
macro_optional_add_subdirectory(kget)
macro_optional_add_subdirectory(kmix)
macro_optional_add_subdirectory(krdc)
macro_optional_add_subdirectory(krfb)
macro_optional_add_subdirectory(ksnapshot)
macro_optional_add_subdirectory(ksystemlog)
macro_optional_add_subdirectory(ktimer)
macro_optional_add_subdirectory(kuser)
macro_optional_add_subdirectory(kvkbd)
macro_optional_add_subdirectory(okular)
macro_optional_add_subdirectory(partitionmanager)
macro_optional_add_subdirectory(thumbnailers)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
