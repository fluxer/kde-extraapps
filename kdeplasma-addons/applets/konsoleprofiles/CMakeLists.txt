project(konsoleprofiles)

set(konsoleprofiles_SRCS
    konsoleprofiles.cpp
)

kde4_add_plugin(plasma_applet_konsoleprofiles ${konsoleprofiles_SRCS})
target_link_libraries(plasma_applet_konsoleprofiles
    KDE4::plasma
)

install(
    TARGETS plasma_applet_konsoleprofiles
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES plasma-applet-konsoleprofiles.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
