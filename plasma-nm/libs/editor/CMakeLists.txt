include_directories(${CMAKE_CURRENT_SOURCE_DIR}/widgets)

set(plasmanetworkmanagement_libeditor_SRCS
    settings/bondwidget.cpp
    settings/bridgewidget.cpp
    settings/btwidget.cpp
    settings/cdmawidget.cpp
    settings/connectionwidget.cpp
    settings/gsmwidget.cpp
    settings/infinibandwidget.cpp
    settings/ipv4widget.cpp
    settings/ipv6widget.cpp
    settings/pppoewidget.cpp
    settings/pppwidget.cpp
    settings/security802-1x.cpp
    settings/vlanwidget.cpp
    settings/wificonnectionwidget.cpp
    settings/wifisecurity.cpp
    settings/wimaxwidget.cpp
    settings/wiredconnectionwidget.cpp
    settings/wiredsecurity.cpp

    widgets/advancedpermissionswidget.cpp
    widgets/bssidcombobox.cpp
    widgets/delegate.cpp
    widgets/hwaddrcombobox.cpp
    widgets/intdelegate.cpp
    widgets/ipv4delegate.cpp
    widgets/ipv4routeswidget.cpp
    widgets/ipv6delegate.cpp
    widgets/ipv6routeswidget.cpp
    widgets/settingwidget.cpp
    widgets/ssidcombobox.cpp

    connectiondetaileditor.cpp
    simpleipv4addressvalidator.cpp
    simpleipv6addressvalidator.cpp
    vpnuiplugin.cpp

    ../uiutils.cpp

    settings/802-1x.ui
    settings/bond.ui
    settings/bridge.ui
    settings/bt.ui
    settings/cdma.ui
    settings/connectionwidget.ui
    settings/gsm.ui
    settings/infiniband.ui
    settings/ipv4.ui
    settings/ipv6.ui
    settings/ppp.ui
    settings/pppoe.ui
    settings/vlan.ui
    settings/wificonnectionwidget.ui
    settings/wifisecurity.ui
    settings/wimax.ui
    settings/wiredconnectionwidget.ui
    settings/wiredsecurity.ui

    widgets/advancedpermissionswidget.ui
    widgets/ipv4routes.ui
    widgets/ipv6routes.ui

    connectiondetaileditor.ui
)

if (WITH_MODEMMANAGER_SUPPORT)
    set(plasmanetworkmanagement_libeditor_SRCS
        ${plasmanetworkmanagement_libeditor_SRCS}
        widgets/mobileconnectionwizard.cpp
        mobileproviders.cpp)
endif()


add_library(plasmanetworkmanagement-editor SHARED ${plasmanetworkmanagement_libeditor_SRCS})
target_link_libraries(plasmanetworkmanagement-editor
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QT_QTXML_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${NETWORKMANAGERQT_LDFLAGS}
    ${MODEMMANAGERQT_LDFLAGS}
)

install(TARGETS plasmanetworkmanagement-editor ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-networkmanagement-vpnuiplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
