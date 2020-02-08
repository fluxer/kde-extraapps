FILE(GLOB lancelot_svgs *.svgz)
install( FILES ${lancelot_svgs}  DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/Androbit/lancelot/ )

FILE(GLOB lancelot_configs *.config)
install( FILES ${lancelot_configs}  DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/Androbit/lancelot/ )

