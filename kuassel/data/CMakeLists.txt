install(FILES kuassel.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES kuassel.notifyrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kuassel)
install(FILES kuasirc.protocol kuasircs.protocol kuasirc6.protocol DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES networks.ini DESTINATION ${KDE4_DATA_INSTALL_DIR}/kuassel)
install(FILES tips DESTINATION  ${KDE4_DATA_INSTALL_DIR}/kuassel )
install(DIRECTORY stylesheets DESTINATION ${KDE4_DATA_INSTALL_DIR}/kuassel)
install(DIRECTORY scripts DESTINATION ${KDE4_DATA_INSTALL_DIR}/kuassel USE_SOURCE_PERMISSIONS)
