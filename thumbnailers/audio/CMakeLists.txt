project(AudioThumbs)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${TAGLIB_CFLAGS} -DHAVE_CONFIG_H=1 -DKDE_NO_COMPAT)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${QT_INCLUDES} ${FLAC_INCLUDES})

set(AudioThumbs_SRCS AudioThumbs.cpp)

kde4_add_plugin(AudioThumbs ${AudioThumbs_SRCS})

target_link_libraries(AudioThumbs ${KDE4_KIO_LIBS} ${TAGLIB_LIBRARIES} ${FLAC_LIBRARIES})

install(TARGETS AudioThumbs DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )

install(FILES  AudioThumbs.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
