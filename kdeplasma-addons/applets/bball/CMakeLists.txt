project(bball)

set(bball_SRCS bball.cpp bballConfig.ui)

kde4_add_plugin(plasma_applet_bball ${bball_SRCS})

target_link_libraries(plasma_applet_bball
    KDE4::plasma
    KDE4::kdeui
    KDE4::kio
)

install(
    TARGETS plasma_applet_bball
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES plasma-applet-bball.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

install(
    FILES bball.png football.png bounce.ogg
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/bball/
)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})
