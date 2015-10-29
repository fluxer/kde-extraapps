project(gwenview)

find_package(KDE4 4.14.3 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(MacroOptionalAddSubdirectory)

## Dependencies
find_package(JPEG)
macro_log_feature(JPEG_FOUND "libjpeg" "JPEG image manipulation support" "http://libjpeg.sourceforge.net/" TRUE)

find_package(PNG)
macro_log_feature(PNG_FOUND "libpng" "PNG image manipulation support" "http://www.libpng.org" TRUE)

find_package(Exiv2)
macro_log_feature(EXIV2_FOUND "Exiv2" "Image metadata support" "http://www.exiv2.org" TRUE "0.19")

macro_optional_find_package(Kipi)
macro_log_feature(KIPI_FOUND "libkipi" "Provides various image manipulation and export features" "http://www.kipi-plugins.org" FALSE)

find_package(LCMS2)
macro_log_feature(LCMS2_FOUND "LittleCMS" "Color management engine" "http://www.littlecms.com" TRUE "2.0")

find_package(LibKonq)
macro_log_feature(LIBKONQ_FOUND "libkonq" "Standard file management features" "http://www.kde.org" TRUE)

## Global settings

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(
    ${QT_DEFINITIONS}
    ${QT_QTDBUS_DEFINITIONS}
    ${KDE4_DEFINITIONS}
    )

include_directories(
    ${QDBUS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${EXIV2_INCLUDE_DIR}
    ${KDCRAW_INCLUDE_DIR}
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
