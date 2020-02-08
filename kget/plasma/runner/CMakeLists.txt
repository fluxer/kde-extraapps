set(krunner_kget_SRCS
    kgetrunner.cpp
    )

qt4_add_dbus_interface(krunner_kget_SRCS ../../dbus/org.kde.kget.main.xml kget_interface)

kde4_add_plugin(krunner_kget
    ${krunner_kget_SRCS}
)
target_link_libraries(krunner_kget
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS
    krunner_kget
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(FILES
    plasma-runner-kget.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
