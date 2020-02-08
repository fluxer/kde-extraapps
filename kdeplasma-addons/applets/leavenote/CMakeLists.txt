# Project Needs a name ofcourse
project(plasma-leavenote)

# We add our source code here
set(leavenote_SRCS leavenote.cpp widget.ui)
kde4_add_plugin(plasma_applet_leavenote ${leavenote_SRCS})

target_link_libraries(plasma_applet_leavenote
                      ${QT_QTNETWORK_LIBRARY} ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_leavenote
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-leavenote.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

# TODO add cmake switch or sth. to only install if the target
# does not exist or reenable with a different svg than notes
# that way we get theming for free since we'd use a copy of
# notes.svgz (leavenote.svgz) otherwise anyway.
#install(
#    FILES notes.svgz
#    DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
