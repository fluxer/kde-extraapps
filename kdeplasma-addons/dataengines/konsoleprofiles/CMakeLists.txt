set(konsoleprofilesengine_SRCS
    konsoleprofilesengine.cpp
    konsoleprofilesservice.cpp
)

kde4_add_plugin(plasma_engine_konsoleprofiles ${konsoleprofilesengine_SRCS})
target_link_libraries(plasma_engine_konsoleprofiles KDE4::plasma KDE4::kio)

install(TARGETS plasma_engine_konsoleprofiles DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-konsoleprofiles.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )

