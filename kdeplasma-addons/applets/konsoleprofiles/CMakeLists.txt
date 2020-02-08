project(konsoleprofiles)

install(DIRECTORY package/
        DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma/plasmoids/konsoleprofiles)

install(FILES package/metadata.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} RENAME plasma-applet-konsoleprofiles.desktop)
