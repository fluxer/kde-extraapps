project(kde-thumbnailer-po)

set(pothumbnail_SRCS pocreator.cpp)

kde4_add_kcfg_files(pothumbnail_SRCS pocreatorsettings.kcfgc)

kde4_add_plugin(pothumbnail ${pothumbnail_SRCS})
target_link_libraries(pothumbnail
    ${KDE4_KIO_LIBS}
    ${GETTEXTPO_LIBRARY}
)

install(TARGETS pothumbnail DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES pocreatorsettings.kcfg DESTINATION ${KDE4_KCFG_INSTALL_DIR})
install(FILES pothumbnail.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
