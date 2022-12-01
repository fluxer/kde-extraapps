project(kde-extraapps)

include(GenerateExportHeader)
include(FeatureSummary)

find_package(KDE4 4.23.0 REQUIRED)
include(KDE4Defaults)

set(LIBRARY_TYPE SHARED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

# global include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
)

kde4_optional_add_subdirectory(about-distro)
kde4_optional_add_subdirectory(ark)
kde4_optional_add_subdirectory(filelight)
kde4_optional_add_subdirectory(gwenview)
kde4_optional_add_subdirectory(kcalc)
kde4_optional_add_subdirectory(kcolorchooser)
kde4_optional_add_subdirectory(kcron)
kde4_optional_add_subdirectory(kdeplasma-addons)
kde4_optional_add_subdirectory(kemu)
kde4_optional_add_subdirectory(kget)
kde4_optional_add_subdirectory(kmix)
kde4_optional_add_subdirectory(krdc)
kde4_optional_add_subdirectory(krfb)
kde4_optional_add_subdirectory(ksnapshot)
kde4_optional_add_subdirectory(ksystemlog)
kde4_optional_add_subdirectory(ktimer)
kde4_optional_add_subdirectory(kuser)
kde4_optional_add_subdirectory(kvkbd)
kde4_optional_add_subdirectory(okular)
kde4_optional_add_subdirectory(partitionmanager)
kde4_optional_add_subdirectory(thumbnailers)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
