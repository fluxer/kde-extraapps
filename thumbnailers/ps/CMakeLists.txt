########### next target ###############

set(gsthumbnail_PART_SRCS gscreator.cpp)

kde4_add_plugin(gsthumbnail ${gsthumbnail_PART_SRCS})

target_link_libraries(gsthumbnail
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTGUI_LIBRARY}
)

install(
    TARGETS gsthumbnail
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES gsthumbnail.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
