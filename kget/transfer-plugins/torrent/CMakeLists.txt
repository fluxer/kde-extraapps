#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/x-bittorrent;" PARENT_SCOPE)

include_directories(
    ../../
    ${CMAKE_CURRENT_BINARY_DIR}/../../
    ${LIBTORRENT_INCLUDES}
)

set(kget_torrentfactory_PART_SRCS
    transferTorrent.cpp
    transferTorrentFactory.cpp
)

kde4_add_plugin(kget_torrentfactory ${kget_torrentfactory_PART_SRCS})

target_link_libraries(kget_torrentfactory
    ${KDE4_KDECORE_LIBS}
    ${LIBTORRENT_LIBRARIES}
    kgetcore
)

set_target_properties(kget_torrentfactory PROPERTIES
    COMPILE_FLAGS "${KDE4_ENABLE_EXCEPTIONS}"
)

install(
    TARGETS kget_torrentfactory
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES kget_torrentfactory.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
