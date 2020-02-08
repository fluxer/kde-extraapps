
include_directories(
    ${PROJECT_BINARY_DIR}/interfaces
    ${PROJECT_SOURCE_DIR}/interfaces
)

########### next target ###############

set(kcm_akrgeneralconfig_SRCS
    akregator_config_general.cpp
    settings_general.ui
)

kde4_add_plugin(akregator_config_general ${kcm_akrgeneralconfig_SRCS})

target_link_libraries(akregator_config_general
    akregatorinterfaces
    ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_general.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_general DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrappearanceconfig_SRCS
    akregator_config_appearance.cpp
    settings_appearance.ui
)

kde4_add_plugin(akregator_config_appearance ${kcm_akrappearanceconfig_SRCS})

target_link_libraries(akregator_config_appearance
    akregatorinterfaces
    ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_appearance.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_appearance DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrarchiveconfig_SRCS 
    akregator_config_archive.cpp
    settings_archive.ui
)

kde4_add_plugin(akregator_config_archive ${kcm_akrarchiveconfig_SRCS})

target_link_libraries(akregator_config_archive
    akregatorinterfaces
    ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_archive.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_archive DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrbrowserconfig_SRCS
    akregator_config_browser.cpp
    settings_browser.ui
)

kde4_add_plugin(akregator_config_browser ${kcm_akrbrowserconfig_SRCS})

target_link_libraries(akregator_config_browser
    akregatorinterfaces
    ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_browser.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_browser DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akradvancedconfig_SRCS
    akregator_config_advanced.cpp
    settings_advanced.cpp
    settings_advancedbase.ui
)

kde4_add_plugin(akregator_config_advanced ${kcm_akradvancedconfig_SRCS})

target_link_libraries(akregator_config_advanced
    akregatorinterfaces
    ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_advanced.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_advanced DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
