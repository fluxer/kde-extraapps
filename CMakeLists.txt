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
    ${CMAKE_CURRENT_BINARY_DIR}/syndication
)

find_package(Boost)
set_package_properties(Boost PROPERTIES
    DESCRIPTION "Free peer-reviewed portable C++ source libraries"
    URL "https://www.boost.org/"
    TYPE RECOMMENDED
    PURPOSE "Required to build syndication and akregator"
)

macro_optional_add_subdirectory(about-distro)
macro_optional_add_subdirectory(ark)
macro_optional_add_subdirectory(bluedevil)
macro_optional_add_subdirectory(dragon)
macro_optional_add_subdirectory(filelight)
macro_optional_add_subdirectory(gwenview)
macro_optional_add_subdirectory(jovie)
macro_optional_add_subdirectory(kcalc)
macro_optional_add_subdirectory(kcron)
macro_optional_add_subdirectory(kdenetwork-filesharing)
macro_optional_add_subdirectory(kdeplasma-addons)
macro_optional_add_subdirectory(libksane)
macro_optional_add_subdirectory(kcolorchooser)
macro_optional_add_subdirectory(kemu)
macro_optional_add_subdirectory(kfloppy)
macro_optional_add_subdirectory(kget)
macro_optional_add_subdirectory(kgpg)
macro_optional_add_subdirectory(kmix)
macro_optional_add_subdirectory(kompare)
macro_optional_add_subdirectory(krdc)
macro_optional_add_subdirectory(krfb)
macro_optional_add_subdirectory(ksaneplugin)
macro_optional_add_subdirectory(ksnapshot)
macro_optional_add_subdirectory(ksystemlog)
macro_optional_add_subdirectory(ktimer)
macro_optional_add_subdirectory(kuser)
macro_optional_add_subdirectory(kvkbd)
macro_optional_add_subdirectory(okular)
macro_optional_add_subdirectory(partitionmanager)
macro_optional_add_subdirectory(skanlite)
macro_optional_add_subdirectory(thumbnailers)
macro_optional_add_subdirectory(yakuake)

# projects that require boost
if(Boost_FOUND)
    macro_optional_add_subdirectory(syndication)
    macro_optional_add_subdirectory(akregator)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
