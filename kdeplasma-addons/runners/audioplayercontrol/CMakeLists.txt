set(krunner_audioplayercontrol_SRCS
    imageiconengine.cpp
    audioplayercontrolrunner.cpp
)

set(kcm_krunner_audioplayercontrol_SRCS
    audioplayercontrolrunner_config.cpp
    audioplayercontrolrunner_config.ui
)

kde4_add_plugin(kcm_krunner_audioplayercontrol ${kcm_krunner_audioplayercontrol_SRCS})
target_link_libraries(kcm_krunner_audioplayercontrol
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

kde4_add_plugin(krunner_audioplayercontrol ${krunner_audioplayercontrol_SRCS})
target_link_libraries(krunner_audioplayercontrol ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})


install(TARGETS krunner_audioplayercontrol kcm_krunner_audioplayercontrol DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-audioplayercontrol.desktop plasma-runner-audioplayercontrol_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
