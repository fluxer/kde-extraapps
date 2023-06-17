project(plasma-dict)

set(dict_SRCS
    dict.cpp
)

kde4_add_plugin(plasma_applet_dict ${dict_SRCS})
target_link_libraries(plasma_applet_dict
    KDE4::plasma
    KDE4::kdeui
)

install(
    TARGETS plasma_applet_dict
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-dict-default.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})
