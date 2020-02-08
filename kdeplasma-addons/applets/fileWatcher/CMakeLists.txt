project(plasma-fileWatcher)
include_directories( ${KDE4_INCLUDES} )

set(fileWatcher_SRCS
    fileWatcher.cpp
    fileWatcherTextItem.cpp
    fileWatcherConfig.ui
    filtersConfig.ui
)

kde4_add_plugin(plasma_applet_fileWatcher ${fileWatcher_SRCS})

target_link_libraries(plasma_applet_fileWatcher ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_fileWatcher DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-fileWatcher-default.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
