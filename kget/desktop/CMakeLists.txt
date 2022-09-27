# associate KGet with the correct mimetypes
configure_file(kget.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kget.desktop)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/kget.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES kget_download.desktop DESTINATION
${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus)

# For DBus activation:
kde4_add_dbus_service(org.kde.kget.service.in)
