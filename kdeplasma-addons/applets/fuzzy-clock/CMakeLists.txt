project(fuzzy-clock)

set(fuzzyclock_SRCS fuzzyClock.cpp fuzzyClockConfig.ui)

kde4_add_plugin(plasma_applet_fuzzy_clock ${fuzzyclock_SRCS})
target_link_libraries(plasma_applet_fuzzy_clock
    KDE4::kdeui
    KDE4::plasma
    KDE4Workspace::plasmaclock
)

install(
    TARGETS plasma_applet_fuzzy_clock
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES plasma-clock-fuzzy.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
