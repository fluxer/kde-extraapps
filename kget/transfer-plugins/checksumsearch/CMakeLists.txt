include_directories(
   ../../
)

set(kget_checksumsearchfactory_PART_SRCS
  checksumsearch.cpp
  checksumsearchtransferdatasource.cpp
  checksumsearchfactory.cpp
)

kde4_add_kcfg_files(kget_checksumsearchfactory_PART_SRCS checksumsearchsettings.kcfgc)

kde4_add_plugin(kget_checksumsearchfactory ${kget_checksumsearchfactory_PART_SRCS})

target_link_libraries(kget_checksumsearchfactory KDE4::kio kgetcore)

install(TARGETS kget_checksumsearchfactory DESTINATION  ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES kget_checksumsearchfactory.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_checksumsearchfactory_PART_SRCS
    checksumsearch.cpp
    dlgchecksumsearch.cpp
    checksumsearch.ui
    checksumsearchadddlg.ui
)

kde4_add_kcfg_files(kcm_kget_checksumsearchfactory_PART_SRCS checksumsearchsettings.kcfgc)
kde4_add_plugin(kcm_kget_checksumsearchfactory ${kcm_kget_checksumsearchfactory_PART_SRCS})
target_link_libraries(kcm_kget_checksumsearchfactory KDE4::kdeui KDE4::kio kgetcore)
install(TARGETS kcm_kget_checksumsearchfactory DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES kget_checksumsearchfactory_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
