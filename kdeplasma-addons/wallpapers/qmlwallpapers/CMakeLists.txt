project(plasma-wallpaper-qml)

kde4_add_plugin(plasma_wallpaper_qml wallpapersmodel.cpp wallpapersqml.cpp backgrounddelegate.cpp viewconfig.ui)
target_link_libraries(plasma_wallpaper_qml
    ${QT_QTDECLARATIVE_LIBRARY}
    KDE4::plasma KDE4::kdecore kdeclarative
)

install(TARGETS plasma_wallpaper_qml DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-qml.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

add_subdirectory(wallpapers)
