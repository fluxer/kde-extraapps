set(potd_engine_SRCS
	cachedprovider.cpp
	potd.cpp
)

kde4_add_plugin(plasma_engine_potd ${potd_engine_SRCS} )
target_link_libraries(plasma_engine_potd plasmapotdprovidercore ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} )

install(TARGETS plasma_engine_potd DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )
install(FILES plasma-dataengine-potd.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )


########### plugin core library ############

set(potd_provider_core_SRCS
	potdprovider.cpp
)

add_library( plasmapotdprovidercore SHARED ${potd_provider_core_SRCS} )
target_link_libraries( plasmapotdprovidercore ${KDE4_KDEUI_LIBS} )

install( TARGETS plasmapotdprovidercore ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES plasma_potdprovider.desktop DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR} )


########### plugins ###############

set( potd_flickr_provider_SRCS
	flickrprovider.cpp
)

kde4_add_plugin( plasma_potd_flickrprovider ${potd_flickr_provider_SRCS} )
target_link_libraries( plasma_potd_flickrprovider plasmapotdprovidercore ${KDE4_KIO_LIBS}  ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
install( TARGETS plasma_potd_flickrprovider DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )
install( FILES flickrprovider.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )


set( potd_apod_provider_SRCS
	apodprovider.cpp
)

kde4_add_plugin( plasma_potd_apodprovider ${potd_apod_provider_SRCS} )
target_link_libraries( plasma_potd_apodprovider plasmapotdprovidercore ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS})
install( TARGETS plasma_potd_apodprovider DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )
install( FILES apodprovider.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )

set( potd_epod_provider_SRCS
	epodprovider.cpp
)

kde4_add_plugin( plasma_potd_epodprovider ${potd_epod_provider_SRCS} )
target_link_libraries( plasma_potd_epodprovider plasmapotdprovidercore ${KDE4_KIO_LIBS} ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY})
install( TARGETS plasma_potd_epodprovider DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )
install( FILES epodprovider.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )
