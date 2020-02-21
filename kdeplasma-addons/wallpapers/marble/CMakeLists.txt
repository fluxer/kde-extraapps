project(plasma-wallpaper-marble)

include_directories(${MARBLE_INCLUDE_DIR})

set(marble_SRCS
    marble.cpp
    config.ui
)

kde4_add_plugin(plasma_wallpaper_marble ${marble_SRCS})

target_link_libraries(plasma_wallpaper_marble
    ${MARBLE_LIBRARIES}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KDECORE_LIBS}
)

install(TARGETS plasma_wallpaper_marble DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-marble.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
