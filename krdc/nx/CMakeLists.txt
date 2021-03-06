
if(LIBNXCL_FOUND)
    add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5013)

    include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
        ${LIBNXCL_INCLUDE_DIR}
    )

    set(nxplugin_SRCS
        nxhostpreferences.cpp
        nxclientthread.cpp
        nxviewfactory.cpp
        nxview.cpp
        nxcallbacks.cpp
        nxresumesessions.cpp
        nxpreferences.ui
        nxresumesessions.ui
    )

    kde4_add_plugin(krdc_nxplugin ${nxplugin_SRCS})

    target_link_libraries(krdc_nxplugin 
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        ${KDE4_KIO_LIBS}
        ${LIBNXCL_LIBRARIES}
        krdccore
    )

    set(kcm_krdc_nxplugin_SRCS
        nxpreferences.cpp
    )

    kde4_add_plugin(kcm_krdc_nxplugin ${kcm_krdc_nxplugin_SRCS})

    target_link_libraries(kcm_krdc_nxplugin
        ${KDE4_KDEUI_LIBS}
        krdccore
    )

    add_dependencies(kcm_krdc_nxplugin krdc_nxplugin)

    install(TARGETS kcm_krdc_nxplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
    install(TARGETS krdc_nxplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

    install(FILES krdc_nx.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
    install(FILES krdc_nx_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

    install(FILES nx.protocol DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
    install(FILES default.dsa.key DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc)
endif(LIBNXCL_FOUND)
