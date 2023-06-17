include_directories(
   ../../
   ${LIBMMS_INCLUDE_DIR}
)

set(kget_mmsfactory_PART_SRCS
  mmsdownload.cpp
  mmstransfer.cpp
  mmstransferfactory.cpp
  mmsthread.cpp
)

kde4_add_kcfg_files(kget_mmsfactory_PART_SRCS mmssettings.kcfgc)
kde4_add_plugin(kget_mmsfactory ${kget_mmsfactory_PART_SRCS})

target_link_libraries(kget_mmsfactory KDE4::kio kgetcore ${LIBMMS_LIBRARIES})

install(TARGETS kget_mmsfactory DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES kget_mmsfactory.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES kget_mmsfactory.kcfg DESTINATION ${KDE4_KCFG_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_mmsfactory_PART_SRCS
  dlgmms.cpp
  dlgmms.ui
)
kde4_add_kcfg_files(kcm_kget_mmsfactory_PART_SRCS mmssettings.kcfgc)
kde4_add_plugin(kcm_kget_mmsfactory ${kcm_kget_mmsfactory_PART_SRCS})
target_link_libraries(kcm_kget_mmsfactory KDE4::kdeui KDE4::kio)
install(TARGETS kcm_kget_mmsfactory DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES kget_mmsfactory_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
