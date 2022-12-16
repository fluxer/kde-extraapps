########### next target ###############

include_directories(
   ${LIBSPECTRE_INCLUDE_DIR}
)

set(psthumbnail_PART_SRCS pscreator.cpp)

kde4_add_kcfg_files(psthumbnail_PART_SRCS pscreatorsettings.kcfgc)
kde4_add_plugin(psthumbnail ${psthumbnail_PART_SRCS})

target_link_libraries(psthumbnail
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${LIBSPECTRE_LIBRARY}
)

install(
    TARGETS psthumbnail
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES psthumbnail.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
