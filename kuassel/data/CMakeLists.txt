install(FILES kuassel.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kuassel.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kuassel)
install(FILES kuasirc.protocol kuasircs.protocol kuasirc6.protocol DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES networks.ini DESTINATION ${DATA_INSTALL_DIR}/kuassel)
install(FILES tips DESTINATION  ${DATA_INSTALL_DIR}/kuassel )
install(DIRECTORY stylesheets DESTINATION ${DATA_INSTALL_DIR}/kuassel)
install(DIRECTORY scripts DESTINATION ${DATA_INSTALL_DIR}/kuassel USE_SOURCE_PERMISSIONS)
