set(openswan_SRCS
    openswan.cpp
    openswanwidget.cpp
    openswanauth.cpp
    openswan.ui 
    openswanauth.ui
)

kde4_add_plugin(plasmanetworkmanagement_openswanui ${openswan_SRCS})

target_link_libraries(plasmanetworkmanagement_openswanui plasmanetworkmanagement-editor ${NETWORKMANAGERQT_LDFLAGS} ${KDE4_KIO_LIBS})

install(TARGETS plasmanetworkmanagement_openswanui  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasmanetworkmanagement_openswanui.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
