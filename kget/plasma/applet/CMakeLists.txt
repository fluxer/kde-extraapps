set(kgetcommonapplet_SRCS
    common/kgetappletutils.cpp
    common/kgetapplet.cpp
)

qt4_add_dbus_interface(kgetcommonapplet_SRCS
    ../../dbus/org.kde.kget.transfer.xml
    transfer_interface
)

set(kgetbarapplet_SRCS
    barapplet/kgetbarapplet.cpp
    ${kgetcommonapplet_SRCS}
)

qt4_add_dbus_interface(kgetbarapplet_SRCS
    ../../dbus/org.kde.kget.main.xml
    kget_interface
)

kde4_add_plugin(plasma_kget_barapplet ${kgetbarapplet_SRCS})
target_link_libraries(plasma_kget_barapplet KDE4::plasma KDE4::kdeui)

install(
    TARGETS plasma_kget_barapplet
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES barapplet/kgetbarapplet-default.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

set(kgetpiechart_SRCS
    piechart/kgetpiechart.cpp
    ${kgetcommonapplet_SRCS}
)

qt4_add_dbus_interface(kgetpiechart_SRCS
    ../../dbus/org.kde.kget.main.xml
    kget_interface
)

kde4_add_plugin(plasma_kget_piechart ${kgetpiechart_SRCS})
target_link_libraries(plasma_kget_piechart KDE4::plasma KDE4::kdeui)

install(
    TARGETS plasma_kget_piechart
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES piechart/kgetpiechartapplet-default.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
