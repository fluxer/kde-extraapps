
if(Q_WS_X11)
    set(HAVE_XFREERDP true)
endif(Q_WS_X11)

if(HAVE_XFREERDP)
    add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5012)

    include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(rdpplugin_SRCS
        rdphostpreferences.cpp
        rdpviewfactory.cpp
        rdpview.cpp
        rdppreferences.ui
    )

    kde4_add_plugin(krdc_rdpplugin ${rdpplugin_SRCS})

    target_link_libraries(krdc_rdpplugin 
        KDE4::kdecore
        KDE4::kdeui
        krdccore
    )

    set(kcm_krdc_rdpplugin_SRCS
        rdppreferences.cpp
    )

    kde4_add_plugin(kcm_krdc_rdpplugin ${kcm_krdc_rdpplugin_SRCS})

    target_link_libraries(kcm_krdc_rdpplugin
        KDE4::kdeui
        krdccore
    )

    add_dependencies(kcm_krdc_rdpplugin krdc_rdpplugin)

    install(TARGETS kcm_krdc_rdpplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
    install(TARGETS krdc_rdpplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

    install(FILES krdc_rdp.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
    install(FILES krdc_rdp_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

    install(FILES rdp.protocol DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
    install(FILES smb2rdc.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus)
endif(HAVE_XFREERDP)
