project(ffmpegthumbs)

include(CheckSymbolExists)
include(CMakePushCheckState)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${FFMPEGTHUMBNAILER_INCLUDES}
)

cmake_reset_check_state()
set(CMAKE_REQUIRED_INCLUDES ${FFMPEGTHUMBNAILER_INCLUDES})
set(CMAKE_REQUIRED_LIBRARIES ${FFMPEGTHUMBNAILER_LIBRARIES})
check_symbol_exists(
    video_thumbnailer_set_size
    "libffmpegthumbnailer/videothumbnailerc.h"
    HAVE_VIDEO_THUMBNAILER_SET_SIZE
)
cmake_reset_check_state()

configure_file (
    config-ffmpegthumbnailer.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-ffmpegthumbnailer.h
)

set(ffmpegthumbs_PART_SRCS
    ffmpegthumbnailer.cpp
)

kde4_add_plugin(ffmpegthumbs ${ffmpegthumbs_PART_SRCS})

target_link_libraries(ffmpegthumbs
    ${KDE4_KIO_LIBS}
    ${FFMPEGTHUMBNAILER_LIBRARIES}
)

install(TARGETS ffmpegthumbs DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES ffmpegthumbs.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
