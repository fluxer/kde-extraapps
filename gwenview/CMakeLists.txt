project(gwenview)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

## Dependencies
find_package(Exiv2 0.19)
set_package_properties(Exiv2 PROPERTIES DESCRIPTION "Image Tag reader"
                       URL "http://www.exiv2.org" TYPE REQUIRED
                       PURPOSE "Support for image metadata")

macro_optional_find_package(Kipi)
macro_log_feature(KIPI_FOUND "libkipi" "Provides various image manipulation and export features" "http://www.kipi-plugins.org" FALSE)

find_package(LibKonq)
macro_log_feature(LIBKONQ_FOUND "libkonq" "Standard file management features" "http://www.kde.org" TRUE)

## Global settings

include_directories(
    ${QDBUS_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
    ${KDCRAW_INCLUDE_DIR}
)

add_definitions(
    ${EXIV2_DEFINITIONS}
    ${KDE4_ENABLE_EXCEPTIONS}
)

## dirs to build
add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(importer)
add_subdirectory(part)
if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(icons)
add_subdirectory(images)
add_subdirectory(cursors)
add_subdirectory(color-schemes)

configure_file(config-gwenview.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-gwenview.h)
