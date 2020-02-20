project(ark)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include(MacroLibrary)

macro_optional_find_package(LibArchive 3.0.3)
set_package_properties(LibArchive PROPERTIES
    DESCRIPTION "A library for dealing with a wide variety of archive file formats"
    URL "http://code.google.com/p/libarchive/"
    PURPOSE "Required for among others tar, tar.gz, tar.bz2 formats in Ark"
)

add_definitions(-DQT_NO_CAST_FROM_ASCII)
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

set(SUPPORTED_ARK_MIMETYPES "")

add_subdirectory(plugins)
add_subdirectory(kerfuffle)
add_subdirectory(part)
add_subdirectory(app)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "Supported MIME types: ${SUPPORTED_ARK_MIMETYPES}")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
