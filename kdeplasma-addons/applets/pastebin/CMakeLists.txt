project(plasma-pastebin)

set(pastebin_SRCS
    pastebin.cpp
    pastebinConfig.ui
)

kde4_add_plugin(plasma_applet_pastebin ${pastebin_SRCS})
target_link_libraries(plasma_applet_pastebin ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_applet_pastebin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-pastebin.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES plasma_pastebin.notifyrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma_pastebin)

