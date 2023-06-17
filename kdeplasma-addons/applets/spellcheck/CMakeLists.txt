set(spellcheck_SRCS SpellCheck.cpp)

kde4_add_plugin(plasma_applet_spellcheck ${spellcheck_SRCS})
target_link_libraries(plasma_applet_spellcheck KDE4::plasma KDE4::kdeui KDE4::kcmutils)

install(TARGETS plasma_applet_spellcheck DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-spellcheck.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
