
# Project needs a name of course
project(blackboard)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )

# We add our source code here
set(blackboard_SRCS blackboard.cpp blackboardwidget.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_blackboard ${blackboard_SRCS})
target_link_libraries(plasma_applet_blackboard
                      KDE4::plasma KDE4::kdeui KDE4::kio)

install(TARGETS plasma_applet_blackboard
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-blackboard.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
