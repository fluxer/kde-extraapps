project(plasma-kolourpicker)

set(kolourpicker_SRCS
    kolourpicker.cpp
)

kde4_add_plugin(plasma_applet_kolourpicker ${kolourpicker_SRCS})
target_link_libraries(plasma_applet_kolourpicker
    KDE4::plasma KDE4::kio
    ${X11_X11_LIB}
)

install(
    TARGETS plasma_applet_kolourpicker
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-kolourpicker-default.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
