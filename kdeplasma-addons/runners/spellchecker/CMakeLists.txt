set(krunner_spellcheckrunner_SRCS
    spellcheck.cpp
)

set(kcm_krunner_spellcheck_SRCS
    spellcheck_config.cpp
    spellcheck_config.ui
)

kde4_add_plugin(kcm_krunner_spellcheck ${kcm_krunner_spellcheck_SRCS})
target_link_libraries(kcm_krunner_spellcheck 
    KDE4::kdecore
    KDE4::kdeui
    KDE4::kcmutils
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

kde4_add_plugin(krunner_spellcheckrunner ${krunner_spellcheckrunner_SRCS})
target_link_libraries(krunner_spellcheckrunner KDE4::kcmutils KDE4::kdeui KDE4::plasma)

install(TARGETS krunner_spellcheckrunner kcm_krunner_spellcheck DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-spellchecker.desktop plasma-runner-spellchecker_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

