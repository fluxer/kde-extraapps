project(nowplaying)

install(DIRECTORY package/
        DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma/plasmoids/nowplaying)

install(FILES package/metadata.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
        RENAME plasma-applet-nowplaying.desktop)

