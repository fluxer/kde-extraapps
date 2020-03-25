project(kfilemetadata)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(PopplerQt4 0.12.1)
set_package_properties(PopplerQt4 PROPERTIES
    DESCRIPTION "A PDF rendering library"
    URL "http://poppler.freedesktop.org"
    PURPOSE "Support for PDF files"
)

macro_optional_find_package(Taglib 1.4)
set_package_properties(Taglib PROPERTIES
    DESCRIPTION "Id3 tag reader"
    URL "https://taglib.org/"
    PURPOSE "Support for music metadata"
)

macro_optional_find_package(Exiv2 0.21)
set_package_properties(Exiv2 PROPERTIES
    DESCRIPTION "Image Tag reader"
    URL "http://www.exiv2.org"
    PURPOSE "Support for image metadata"
)

macro_optional_find_package(FFmpeg 1.0)
set_package_properties(FFmpeg PROPERTIES
    DESCRIPTION "Video Tag reader"
    URL "http://ffmpeg.org"
    PURPOSE "Support for video metadata"
)

macro_optional_find_package(EPub)
set_package_properties(EPub PROPERTIES
    DESCRIPTION "A library for reading EPub documents"
    URL "http://sourceforge.net/projects/ebook-tools"
    PURPOSE "Support for epub metadata"
)

macro_optional_find_package(QMobipocket)
set_package_properties(QMobipocket PROPERTIES
    DESCRIPTION "A library for reading Mobipocket documents"
    URL "https://projects.kde.org/projects/kde/kdegraphics/kdegraphics-mobipocket"
    PURPOSE "Support for Mobipocket documents metadata in KFileMetaData"
)

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src
)

add_subdirectory(src)
if(ENABLE_TESTING)
    add_subdirectory(autotests)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/KFileMetaDataConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KFileMetaDataConfig.cmake
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/KFileMetaDataConfig.cmake
    DESTINATION ${KDE4_LIB_INSTALL_DIR}/cmake/KFileMetaData
)
