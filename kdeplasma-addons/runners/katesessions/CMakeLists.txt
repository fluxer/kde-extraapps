set(krunner_katesessions_SRCS
    katesessions.cpp
)

kde4_add_plugin(krunner_katesessions ${krunner_katesessions_SRCS})
target_link_libraries(krunner_katesessions KDE4::plasma KDE4::kio)

install(TARGETS krunner_katesessions DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )

install(FILES katesessions.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

