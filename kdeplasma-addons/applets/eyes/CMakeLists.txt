project(eyes)

 
set(eyes_SRCS eyes.cpp)

kde4_add_plugin(plasma_applet_eyes ${eyes_SRCS})
target_link_libraries(plasma_applet_eyes
                      KDE4::plasma KDE4::kdeui)
 
install(TARGETS plasma_applet_eyes
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-eyes.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES eyes.svg
	DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})
