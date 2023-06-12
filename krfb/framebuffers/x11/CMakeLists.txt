include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(krfb_framebuffer_x11_SRCS
    x11framebuffer.cpp
    x11framebufferplugin.cpp
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/x11framebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/x11framebufferplugin.cpp
    PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
)

kde4_add_plugin(krfb_framebuffer_x11
    ${krfb_framebuffer_x11_SRCS}
)

target_link_libraries(krfb_framebuffer_x11
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${X11_X11_LIB}
    ${X11_Xdamage_LIB}
    ${X11_Xext_LIB}
    krfbprivate
)

install(
    TARGETS krfb_framebuffer_x11
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

install(
    FILES krfb_framebuffer_x11.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

