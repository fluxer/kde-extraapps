project(plasmaconverter)

set(krunner_converter_SRCS
    converterrunner.cpp
)

kde4_add_plugin(krunner_converter ${krunner_converter_SRCS})
target_link_libraries(krunner_converter ${KDE4_KUNITCONVERSION_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS})
install(TARGETS krunner_converter DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-converter.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

