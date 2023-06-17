project(plasma-wallpaper-virus)

set(virus_SRCS
    virus.cpp
    backgrounddelegate.cpp
    backgroundlistmodel.cpp
    alife.cpp
    itemsview.cpp
)

if(KDE4WORKSPACE_FOUND)
    add_definitions(-DHAVE_KWORKSPACE)
    set(ksmserver_xml ${KDE4_DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSMServerInterface.xml)
    qt4_add_dbus_interface(virus_SRCS ${ksmserver_xml} ksmserver_interface)
endif()

kde4_add_plugin(plasma_wallpaper_virus ${virus_SRCS})
target_link_libraries(plasma_wallpaper_virus
    KDE4::plasma
    KDE4::kio
    KDE4::kfile
)

install(
    TARGETS plasma_wallpaper_virus
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-wallpaper-virus.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
