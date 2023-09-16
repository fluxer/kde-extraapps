set(potd_engine_SRCS
    cachedprovider.cpp
    potd.cpp
)

kde4_add_plugin(plasma_engine_potd ${potd_engine_SRCS})
target_link_libraries(plasma_engine_potd
    KDE4::kdeui
    KDE4::plasma
    plasmapotdprovidercore
)

install(
    TARGETS plasma_engine_potd
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES plasma-dataengine-potd.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

########### plugin core library ############

set(potd_provider_core_SRCS
    potdprovider.cpp
)

add_library(plasmapotdprovidercore SHARED ${potd_provider_core_SRCS})
target_link_libraries(plasmapotdprovidercore KDE4::kdeui)

generate_export_header(plasmapotdprovidercore BASE_NAME plasma_potd)

install(
    TARGETS plasmapotdprovidercore
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
install(
    FILES plasma_potdprovider.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

########### plugins ###############

set( potd_flickr_provider_SRCS
    flickrprovider.cpp
)

kde4_add_plugin(plasma_potd_flickrprovider ${potd_flickr_provider_SRCS})
target_link_libraries(plasma_potd_flickrprovider
    KDE4::kio
    KDE4::kdecore
    ${QT_QTGUI_LIBRARY}
    plasmapotdprovidercore
)
install(
    TARGETS plasma_potd_flickrprovider
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES flickrprovider.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)


set(potd_apod_provider_SRCS
    apodprovider.cpp
)

kde4_add_plugin(plasma_potd_apodprovider ${potd_apod_provider_SRCS})
target_link_libraries(plasma_potd_apodprovider
    KDE4::kio
    KDE4::kdecore
    ${QT_QTGUI_LIBRARY}
    plasmapotdprovidercore 
)
install(
    TARGETS plasma_potd_apodprovider
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES apodprovider.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)


set(potd_pexels_provider_SRCS
    pexelsprovider.cpp
)

kde4_add_plugin(plasma_potd_pexelsprovider ${potd_pexels_provider_SRCS})
target_link_libraries(plasma_potd_pexelsprovider
    KDE4::kio
    KDE4::kdecore
    ${QT_QTGUI_LIBRARY}
    plasmapotdprovidercore 
)
install(
    TARGETS plasma_potd_pexelsprovider
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES pexelsprovider.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
