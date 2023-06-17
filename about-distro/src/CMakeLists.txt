include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kcm_SRCS
    main.cpp
    Module.cpp
    OSRelease.cpp
    Module.ui
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Module.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
)

kde4_add_plugin(kcm_about_distro ${kcm_SRCS})

target_link_libraries(kcm_about_distro
    KDE4::kdeui
    KDE4::kio
    KDE4::solid
)

install(
    TARGETS kcm_about_distro
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES about-distro.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
