
set (upnp_HDR
	upnprouter.h
	upnpmcastsocket.h
)

install(FILES ${upnp_HDR} DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/libktorrent/upnp COMPONENT Devel)