set(printmanager_config_SRCS
    KPrintManagerConfigPlugin.cpp
)

kde4_add_plugin(printmanagerwidget ${printmanager_config_SRCS})

target_link_libraries(printmanagerwidget
    ${KDE4_KDEUI_LIBS}
    kcupslib
)

install(TARGETS printmanagerwidget DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}/plugins/designer)

# QML Stuff
install(FILES package/metadata.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} RENAME plasma-applet-printmanager.desktop)
install(DIRECTORY package/ DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma/plasmoids/org.kde.printmanager)
