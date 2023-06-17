set(krunner_konsolesessions_SRCS
    konsolesessions.cpp
)

kde4_add_plugin(krunner_konsolesessions ${krunner_konsolesessions_SRCS})
target_link_libraries(krunner_konsolesessions KDE4::plasma KDE4::kio)

install(TARGETS krunner_konsolesessions DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )

install(FILES konsolesessions.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

