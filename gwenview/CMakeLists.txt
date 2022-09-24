project(gwenview)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.22.0 REQUIRED)
    include(KDE4Defaults)
    include(GenerateExportHeader)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

## Dependencies
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
set_package_properties(Threads PROPERTIES
    PURPOSE "Required for asynchorouns functions and methods execution"
    DESCRIPTION "Platform dependent threads implementation"
    URL ""
    TYPE REQUIRED
)

find_package(Exiv2 0.19)
set_package_properties(Exiv2 PROPERTIES
    DESCRIPTION "Image Tag reader"
    URL "http://www.exiv2.org"
    PURPOSE "Support for image metadata"
    TYPE REQUIRED
)

find_package(LibKonq)
set_package_properties(LibKonq PROPERTIES
    URL "https://osdn.net/projects/kde/"
    PURPOSE "Standard file management features"
    TYPE REQUIRED
)

## Global settings

include_directories(
    ${QDBUS_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
    # for gwenview_export.h
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/lib
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
