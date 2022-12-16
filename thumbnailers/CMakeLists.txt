project(kdegraphics-thumbnailers)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDE4 4.23.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

kde4_optional_find_package(Taglib 1.4)
set_package_properties(Taglib PROPERTIES
    DESCRIPTION "Id3 tag reader"
    URL "https://taglib.org/"
    PURPOSE "Required to build the audio thumbnailer"
)

kde4_optional_find_package(FFmpegThumbnailer)
set_package_properties(FFmpegThumbnailer PROPERTIES
    DESCRIPTION "Lightweight video thumbnailer that can be used by file managers to create thumbnails for video files"
    URL "https://github.com/dirkvdb/ffmpegthumbnailer"
    PURPOSE "Required to build the video thumbnailer"
)

kde4_optional_find_package(Poppler)
set_package_properties(Poppler PROPERTIES
    DESCRIPTION "A PDF rendering library"
    URL "https://poppler.freedesktop.org"
    PURPOSE "Required to build the PDF thumbnailer"
)

kde4_optional_find_package(GettextPO)
set_package_properties(GettextPO PROPERTIES
    DESCRIPTION "A library for processing PO files"
    URL "www.gnu.org/software/gettext"
    PURPOSE "Required to build the PO thumbnailer"
)

kde4_optional_find_package(LibSpectre)
set_package_properties(LibSpectre PROPERTIES
    DESCRIPTION "Small library for rendering Postscript documents"
    URL "https://www.freedesktop.org/wiki/Software/libspectre/"
    PURPOSE "Required to build the Postscript thumbnailer"
)

if(TAGLIB_FOUND)
    add_subdirectory(audio)
endif()

if(FFMPEGTHUMBNAILER_FOUND)
    add_subdirectory(ffmpegthumbs)
endif()

if(Poppler_FOUND)
    add_subdirectory(pdf)
endif()

if(GETTEXTPO_FOUND)
    add_subdirectory(po)
endif()

if (LIBSPECTRE_FOUND)
    add_subdirectory(ps)
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
