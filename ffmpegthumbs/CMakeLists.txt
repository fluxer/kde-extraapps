project(ffmpegthumbs)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE)

include_directories(
    ${PC_AVCODEC_INCLUDEDIR}
    ${PC_AVFORMAT_INCLUDEDIR}
    ${FFMPEG_INCLUDE_DIR}
)

# Certain versions of FFMPEG need this to be defined
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D __STDC_CONSTANT_MACROS")

set(ffmpegthumbs_PART_SRCS
    ffmpegthumbnailer.cpp
    ffmpegthumbnailer/filmstripfilter.cpp
    ffmpegthumbnailer/moviedecoder.cpp
    ffmpegthumbnailer/imagewriter.cpp
    ffmpegthumbnailer/videothumbnailer.cpp
)

kde4_add_plugin(ffmpegthumbs ${ffmpegthumbs_PART_SRCS})

target_link_libraries(ffmpegthumbs
    ${KDE4_KIO_LIBS}
    ${AVUTIL_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVCODEC_LIBRARIES}
    ${SWSCALE_LIBRARIES}
)

install(TARGETS ffmpegthumbs DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES ffmpegthumbs.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
