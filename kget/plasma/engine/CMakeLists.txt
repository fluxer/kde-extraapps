include_directories( ../../)

set(kget_engine_SRCS
    kgetengine.cpp
)

qt4_add_dbus_interface(kget_engine_SRCS ../../dbus/org.kde.kget.main.xml kget_interface)

kde4_add_plugin(plasma_engine_kget ${kget_engine_SRCS})
target_link_libraries(plasma_engine_kget KDE4::kdecore KDE4::plasma)

install(TARGETS plasma_engine_kget DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-kget.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
