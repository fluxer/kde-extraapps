include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kcm_SRCS
    main.cpp
    Module.cpp
    OSRelease.cpp
    Module.ui
)

kde4_add_plugin(kcm_about_distro ${kcm_SRCS})

target_link_libraries(kcm_about_distro
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
)

install(
    TARGETS kcm_about_distro
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES about-distro.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
