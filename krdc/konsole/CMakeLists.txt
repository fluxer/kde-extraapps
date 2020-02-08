
set(BUILD_KONSOLE true)
if(BUILD_KONSOLE)
    set(konsoleplugin_SRCS
        konsoleviewfactory.cpp
        konsoleview.cpp
    )

    kde4_add_plugin(krdc_konsoleplugin ${konsoleplugin_SRCS})

    target_link_libraries(krdc_konsoleplugin 
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KPARTS_LIBS}
        krdccore
    )
    install(TARGETS krdc_konsoleplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

    install(FILES krdc_konsole.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
endif(BUILD_KONSOLE)
