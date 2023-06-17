set(krunner_audioplayercontrol_SRCS
    audioplayercontrolrunner.cpp
)

set(kcm_krunner_audioplayercontrol_SRCS
    audioplayercontrolrunner_config.cpp
    audioplayercontrolrunner_config.ui
)

kde4_add_plugin(kcm_krunner_audioplayercontrol ${kcm_krunner_audioplayercontrol_SRCS})
target_link_libraries(kcm_krunner_audioplayercontrol
    KDE4::kdecore
    KDE4::kdeui
    KDE4::kcmutils
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

kde4_add_plugin(krunner_audioplayercontrol ${krunner_audioplayercontrol_SRCS})
target_link_libraries(krunner_audioplayercontrol KDE4::kio KDE4::kdeui KDE4::plasma)


install(TARGETS krunner_audioplayercontrol kcm_krunner_audioplayercontrol DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-audioplayercontrol.desktop plasma-runner-audioplayercontrol_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
