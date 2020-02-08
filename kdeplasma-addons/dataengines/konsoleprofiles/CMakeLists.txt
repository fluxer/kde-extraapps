set(konsoleprofilesengine_SRCS
    konsoleprofilesengine.cpp
    konsoleprofilesservice.cpp
)

kde4_add_plugin(plasma_engine_konsoleprofiles ${konsoleprofilesengine_SRCS})
target_link_libraries(plasma_engine_konsoleprofiles ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_engine_konsoleprofiles DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-konsoleprofiles.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )
install(FILES org.kde.plasma.dataengine.konsoleprofiles.operations DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma/services)

