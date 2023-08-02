
# Project needs a name of course
project(timer)

# We add our source code here
set(timer_SRCS
    timer.cpp
    timerdigit.cpp
    customtimeeditor.cpp
    timerConfig.ui
    predefinedTimersConfig.ui
)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_timer ${timer_SRCS})
target_link_libraries(plasma_applet_timer
                      KDE4::plasma KDE4::kdeui)

install(TARGETS plasma_applet_timer
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-timer.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES timer.svgz
        DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
