project(plasma-showdesktop)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} )

set(showdesktop_SRCS
    showdesktop.cpp
)

kde4_add_plugin(plasma_applet_showdesktop ${showdesktop_SRCS})
target_link_libraries(plasma_applet_showdesktop
     KDE4::kdeui KDE4::plasma )

install(TARGETS plasma_applet_showdesktop DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-showdesktop.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
