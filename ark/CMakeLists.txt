project(ark)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(GenerateExportHeader)
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

kde4_optional_find_package(LibArchive 3.0.3)
set_package_properties(LibArchive PROPERTIES
    DESCRIPTION "Multi-format archive and compression library"
    URL "https://libarchive.org/"
    PURPOSE "Required for among others tar, tar.gz, tar.bz2 formats in Ark"
)

kde4_optional_find_package(KDE4Workspace)
set_package_properties(KDE4Workspace PROPERTIES
    DESCRIPTION "KDE base workspace libraries"
    URL "https://osdn.net/projects/kde/"
    PURPOSE "Needed for popup menu plugin"
)

kde4_optional_find_package(LibDeflate)
set_package_properties(LibDeflate PROPERTIES
    DESCRIPTION "Heavily optimized library for DEFLATE/zlib/gzip compression and decompression"
    URL "https://github.com/ebiggers/libdeflate"
    PURPOSE "Required for the .gz format support in Ark"
)

kde4_optional_find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
    DESCRIPTION "A high-quality data compressor"
    URL "http://www.bzip.org"
    PURPOSE "Required for the .bz2 format support in Ark"
)

kde4_optional_find_package(LibLZMA)
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
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
