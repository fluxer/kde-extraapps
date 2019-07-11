project(kdegraphics-thumbnailers)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.18.0 REQUIRED)
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

macro_optional_find_package(Taglib)
set_package_properties(Taglib PROPERTIES
    DESCRIPTION "Id3 tag reader"
    URL "http://developer.kde.org/~wheeler/taglib.html"
    PURPOSE "Required to build the audio thumbnailer"
)

macro_optional_find_package(FLAC++)
set_package_properties(FLAC++ PROPERTIES
    DESCRIPTION "Free Lossless Audio Codec"
    URL "https://xiph.org/flac/"
    PURPOSE "Required to build the audio thumbnailer"
)

if(TAGLIB_FOUND AND FLAC_FOUND)
    macro_optional_add_subdirectory(audio)
endif()
macro_optional_add_subdirectory(ps)
macro_optional_add_subdirectory(raw)

if(GETTEXTPO_FOUND)
    macro_optional_add_subdirectory(po)
endif(GETTEXTPO_FOUND)

