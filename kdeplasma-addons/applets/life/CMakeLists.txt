project(life)

set(life_SRCS life.cpp lifeConfig.ui)
kde4_add_plugin(plasma_applet_life ${life_SRCS})
target_link_libraries(plasma_applet_life
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_life
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-life.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})
