project(binaryclock)

set(binaryclock_SRCS binaryclock.cpp clockConfig.ui)

kde4_add_plugin(plasma_applet_binaryclock ${binaryclock_SRCS})

target_link_libraries(plasma_applet_binaryclock
    KDE4::kdeui
    KDE4::plasma
    KDE4Workspace::plasmaclock
)

install(
    TARGETS plasma_applet_binaryclock
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES plasma-applet-binaryclock.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
