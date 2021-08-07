set(krunner_kemu_SRCS
    krunner_kemu.cpp
)

kde4_add_plugin(krunner_kemu ${krunner_kemu_SRCS})
target_link_libraries(krunner_kemu
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
)

install(
    TARGETS krunner_kemu
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES plasma-runner-kemu.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
