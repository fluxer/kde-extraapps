include_directories(${SCIM_INCLUDE_DIR})

set(SCIM_ICON_DIR "${PC_SCIM_PREFIX}/share/scim/icons")
configure_file(config-scim.h.cmake config-scim.h)

set(kimpanel_scim_panel_SRCS main.cpp)
add_executable(kimpanel-scim-panel ${kimpanel_scim_panel_SRCS})
target_link_libraries(kimpanel-scim-panel
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${SCIM_LIBRARIES}
)

install(TARGETS kimpanel-scim-panel DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})
