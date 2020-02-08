########### next target ###############

set(jovie_talkerchooserplugin_PART_SRCS 
    selecttalkerdlg.cpp
    talkerchooserconf.cpp 
    talkerchooserproc.cpp 
    talkerchooserplugin.cpp
)

kde4_add_plugin(jovie_talkerchooserplugin ${jovie_talkerchooserplugin_PART_SRCS})

target_link_libraries(jovie_talkerchooserplugin ${KDE4_KIO_LIBS} kttsd)

install(
    TARGETS jovie_talkerchooserplugin
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES female_notifications_rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/jovie/talkerchooser
)

install(
    FILES jovie_talkerchooserplugin.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
