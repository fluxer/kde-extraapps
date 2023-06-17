project(luna)

set(luna_SRCS luna.cpp lunaConfig.ui)
kde4_add_plugin(plasma_applet_luna ${luna_SRCS})
target_link_libraries(plasma_applet_luna
                      KDE4::plasma KDE4::kdeui)
 
install(TARGETS plasma_applet_luna
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-luna.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES luna.svgz
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})
