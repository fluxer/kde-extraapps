set(kded_kemu_SRCS 
    kded_kemu.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kemu.xml
)

qt4_generate_dbus_interface(kded_kemu.h org.kde.kemu.xml)

kde4_add_plugin(kded_kemu ${kded_kemu_SRCS})
target_link_libraries(kded_kemu KDE4::kdecore)

install(
    TARGETS kded_kemu
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES kemu.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kemu.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)


