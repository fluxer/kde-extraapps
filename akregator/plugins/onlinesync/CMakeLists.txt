include_directories(
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/src
  ${CMAKE_SOURCE_DIR}
  ${KDE4_INCLUDE_DIR}
  ${QT_INCLUDES}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(kcm_akronlinesyncconfig_SRCS
    akregator_config_onlinesync.cpp
    ui/configurationdialog.cpp
    ui/configurationwidget.cpp
    ui/configurationdialogadd.cpp
    ui/configurationwidget.ui
    ui/configurationdialogadd.ui
)

kde4_add_kcfg_files(kcm_akronlinesyncconfig_SRCS onlinesyncsettings.kcfgc)

kde4_add_plugin(akregator_config_onlinesync ${kcm_akronlinesyncconfig_SRCS})

target_link_libraries(akregator_config_onlinesync
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
  ${KDE4_KPARTS_LIBS}
)

install(FILES akregator_config_onlinesync.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_onlinesync DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### next target ###############

set(akregator_onlinesync_plugin_SRCS
    sync/feedsync.cpp
    sync/subscriptionlist.cpp
    sync/akregator.cpp
    sync/googlereader.cpp
    sync/opml.cpp
    ui/configurationdialog.cpp
    ui/configurationwidget.cpp
    ui/configurationdialogadd.cpp
    onlinesyncplugin.cpp
    ui/configurationwidget.ui
    ui/configurationdialogadd.ui
)

kde4_add_kcfg_files(akregator_onlinesync_plugin_SRCS onlinesyncsettings.kcfgc)

kde4_add_plugin(akregator_onlinesync_plugin ${akregator_onlinesync_plugin_SRCS})

target_link_libraries(akregator_onlinesync_plugin
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
  ${KDE4_KPARTS_LIBS}
)

install(FILES akregator_onlinesync_plugin.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(TARGETS akregator_onlinesync_plugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES akregator_onlinesync_plugin.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/akregator_onlinesync_plugin)
