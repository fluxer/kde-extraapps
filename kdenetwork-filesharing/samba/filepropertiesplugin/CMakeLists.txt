########### next target ###############

set(sambausershareplugin_PART_SRCS
    sambausershareplugin.cpp
    delegate.cpp
    model.cpp
    sambausershareplugin.ui
)

kde4_add_plugin(sambausershareplugin ${sambausershareplugin_PART_SRCS})

target_link_libraries(sambausershareplugin ${KDE4_KIO_LIBS})

install(TARGETS sambausershareplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES sambausershareplugin.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
