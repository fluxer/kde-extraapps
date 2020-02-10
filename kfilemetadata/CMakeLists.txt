project(kfilemetadata)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(PopplerQt4 0.12.1)
set_package_properties(PopplerQt4 PROPERTIES DESCRIPTION "A PDF rendering library"
                       URL "http://poppler.freedesktop.org" TYPE OPTIONAL
                       PURPOSE "Support for PDF files")

find_package(Taglib)
set_package_properties(Taglib PROPERTIES DESCRIPTION "Id3 tag reader"
                       URL "http://developer.kde.org/~wheeler/taglib.html" TYPE OPTIONAL
                       PURPOSE "Support for music metadata")

find_package(Exiv2 0.21)
set_package_properties(Exiv2 PROPERTIES DESCRIPTION "Image Tag reader"
                       URL "http://www.exiv2.org" TYPE OPTIONAL
                       PURPOSE "Support for image metadata")

find_package(FFmpeg 1.0)
set_package_properties(FFmpeg PROPERTIES DESCRIPTION "Video Tag reader"
                       URL "http://ffmpeg.org" TYPE OPTIONAL
                       PURPOSE "Support for video metadata")

find_package(EPub)
set_package_properties(EPub PROPERTIES DESCRIPTION "Ebook epub reader"
                       URL "http://sourceforge.net/projects/ebook-tools" TYPE OPTIONAL
                       PURPOSE "Support for epub metadata")

find_package(QMobipocket)
set_package_properties(QMobipocket PROPERTIES DESCRIPTION "Mobipocket epub reader"
                       URL "https://projects.kde.org/projects/kde/kdegraphics/kdegraphics-mobipocket"
                       TYPE OPTIONAL PURPOSE "Support for mobi metadata")

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src
)

add_subdirectory(src)
if(ENABLE_TESTING)
    add_subdirectory(autotests)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/KFileMetaDataConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/KFileMetaDataConfig.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KFileMetaDataConfig.cmake
        DESTINATION ${KDE4_LIB_INSTALL_DIR}/cmake/KFileMetaData)
