project(plasma-applet-bubblemon)

add_subdirectory(src)
add_subdirectory(images)

install(FILES plasma-applet-bubblemon.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
