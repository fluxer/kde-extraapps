project(ffmpegthumbs)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.14.3 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})

    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
    add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
endif()

find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT SWSCALE)

include_directories(
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
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

install(TARGETS ffmpegthumbs DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES ffmpegthumbs.desktop DESTINATION ${SERVICES_INSTALL_DIR})

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
