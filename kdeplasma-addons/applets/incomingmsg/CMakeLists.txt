# Project Needs a name ofcourse
project(plasma-incomingmsg)

# We add our source code here
set(incomingmsg_SRCS incomingmsg.cpp widget.ui)
kde4_add_plugin(plasma_applet_incomingmsg ${incomingmsg_SRCS})

target_link_libraries(plasma_applet_incomingmsg
                      ${QT_QTNETWORK_LIBRARY} KDE4::plasma KDE4::kdeui)

install(TARGETS plasma_applet_incomingmsg
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-incomingmsg.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
