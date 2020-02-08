add_subdirectory( icons )
add_subdirectory( skins )

install(PROGRAMS yakuake.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

install(FILES yakuake.notifyrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/yakuake)

