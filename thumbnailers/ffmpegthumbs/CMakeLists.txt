project(ffmpegthumbs)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(
    ${FFMPEGTHUMBNAILER_INCLUDES}
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
