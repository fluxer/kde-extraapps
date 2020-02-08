set(kerfuffle_cli_SRCS cliplugin.cpp)
kde4_add_plugin(kerfuffle_cli ${kerfuffle_cli_SRCS})
target_link_libraries(kerfuffle_cli ${KDE4_KDECORE_LIBS} kerfuffle)

install(TARGETS kerfuffle_cli DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES kerfuffle_cli.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

