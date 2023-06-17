project(ffmpegthumbs)

include(CMakePushCheckState)
include(CheckSymbolExists)
include(CheckStructHasMember)

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

check_struct_has_member(
    "video_thumbnailer"
    "prefer_embedded_metadata"
    "libffmpegthumbnailer/videothumbnailerc.h"
    HAVE_PREFER_EMBEDDED_METADATA
)
cmake_reset_check_state()

configure_file (
    config-ffmpegthumbnailer.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-ffmpegthumbnailer.h
)

set(ffmpegthumbs_PART_SRCS
    ffmpegthumbnailer.cpp
)

kde4_add_kcfg_files(ffmpegthumbs_PART_SRCS ffmpegthumbssettings.kcfgc)
kde4_add_plugin(ffmpegthumbs ${ffmpegthumbs_PART_SRCS})

target_link_libraries(ffmpegthumbs
    KDE4::kio
    ${FFMPEGTHUMBNAILER_LIBRARIES}
)

install(TARGETS ffmpegthumbs DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES ffmpegthumbs.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
