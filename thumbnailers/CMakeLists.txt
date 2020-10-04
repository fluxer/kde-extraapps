project(kdegraphics-thumbnailers)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(GettextPO)
set_package_properties(GettextPO PROPERTIES
    DESCRIPTION "A library for processing PO files"
    URL "www.gnu.org/software/gettext"
    PURPOSE "Required to build the PO thumbnailer"
)

macro_optional_find_package(Taglib 1.4)
set_package_properties(Taglib PROPERTIES
    DESCRIPTION "Id3 tag reader"
    URL "https://taglib.org/"
    PURPOSE "Required to build the audio thumbnailer"
)

macro_optional_find_package(FLAC++)
set_package_properties(FLAC++ PROPERTIES
    DESCRIPTION "Free Lossless Audio Codec"
    URL "https://xiph.org/flac/"
    PURPOSE "Required to build the audio thumbnailer"
)

macro_optional_find_package(FFmpegThumbnailer)
set_package_properties(FFmpegThumbnailer PROPERTIES
    DESCRIPTION "Lightweight video thumbnailer that can be used by file managers to create thumbnails for video files"
    URL "https://github.com/dirkvdb/ffmpegthumbnailer"
    PURPOSE "Required to build the video thumbnailer"
)

if(TAGLIB_FOUND AND FLAC_FOUND)
    macro_optional_add_subdirectory(audio)
endif()

macro_optional_add_subdirectory(ps)

macro_optional_add_subdirectory(raw)

if(GETTEXTPO_FOUND)
    macro_optional_add_subdirectory(po)
endif(GETTEXTPO_FOUND)

if(FFMPEGTHUMBNAILER_FOUND)
    macro_optional_add_subdirectory(ffmpegthumbs)
endif()
