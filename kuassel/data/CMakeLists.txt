if (KDE4_FOUND OR (UNIX AND NOT APPLE))
    install(FILES quassel.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})

    install(FILES quassel.notifyrc DESTINATION ${DATA_INSTALL_DIR}/quassel)
endif()

install(FILES networks.ini DESTINATION ${DATA_INSTALL_DIR}/quassel)
install(DIRECTORY stylesheets DESTINATION ${DATA_INSTALL_DIR}/quassel)
install(DIRECTORY scripts DESTINATION ${DATA_INSTALL_DIR}/quassel USE_SOURCE_PERMISSIONS)
