set(bubblemon_SRCS
    bubble.cpp
    settings.ui
)

kde4_add_plugin(plasma_applet_bubblemon ${bubblemon_SRCS})
target_link_libraries(plasma_applet_bubblemon KDE4::plasma)

install(TARGETS plasma_applet_bubblemon DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
