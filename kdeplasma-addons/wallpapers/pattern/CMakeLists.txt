project(plasma-wallpaper-pattern)

set(pattern_SRCS
    pattern.cpp
    backgrounddelegate.cpp
    backgroundlistmodel.cpp
    itemsview.cpp
    config.ui
)

kde4_add_plugin(plasma_wallpaper_pattern ${pattern_SRCS})
target_link_libraries(plasma_wallpaper_pattern ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KFILE_LIBS})

install(TARGETS plasma_wallpaper_pattern DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-pattern.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})


FILE(GLOB pattern_desktop patterns/*.desktop)
FILE(GLOB pattern_png patterns/*.jpg)
FILE(GLOB pattern_jpg patterns/*.png)

INSTALL(FILES ${pattern_desktop}  ${pattern_png} ${pattern_jpg} DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma_wallpaper_pattern/patterns)
