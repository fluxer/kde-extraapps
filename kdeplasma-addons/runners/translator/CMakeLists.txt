set (krunner_translator_SRCS
    translator.cpp
    translatorjob.cpp
)

kde4_add_plugin(krunner_translator ${krunner_translator_SRCS})
target_link_libraries(krunner_translator
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
)

install(
    TARGETS krunner_translator
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-runner-translator.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
