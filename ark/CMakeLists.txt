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

macro_optional_find_package(LibKonq)
set_package_properties(LibKonq PROPERTIES
    DESCRIPTION "libkonq library"
    PURPOSE "Need to integrate in konqueror"
)

macro_optional_find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "General purpose data compression library"
    URL "http://www.zlib.net"
    PURPOSE "Required for the .gz format support in Ark"
)

macro_optional_find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
    DESCRIPTION "A high-quality data compressor"
    URL "http://www.bzip.org"
    PURPOSE "Required for the .bz2 format support in Ark"
)

macro_optional_find_package(LibLZMA)
set_package_properties(LibLZMA PROPERTIES
    DESCRIPTION "Legacy data compression software with high compression ratio"
    URL "http://tukaani.org/xz/"
    PURPOSE "Required for the .xz and .lzma format support in Ark"
)

add_definitions(-DQT_NO_CAST_FROM_ASCII)
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

set(SUPPORTED_ARK_MIMETYPES "")

include_directories(
    # for kerfuffle_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle
)

add_subdirectory(plugins)
add_subdirectory(kerfuffle)
add_subdirectory(part)
add_subdirectory(app)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    message(STATUS "Supported MIME types: ${SUPPORTED_ARK_MIMETYPES}")
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
