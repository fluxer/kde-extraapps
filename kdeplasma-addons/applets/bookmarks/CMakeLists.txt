project( plasma-bookmarks )

set( bookmarks_SRCS
    generalconfigeditor.cpp
    bookmarksplasmoid.cpp
    main.cpp
)

kde4_add_plugin( plasma_applet_bookmarks  ${bookmarks_SRCS})

target_link_libraries( plasma_applet_bookmarks
    KDE4::plasma
    KDE4::kdeui
    KDE4::kio
)

install( TARGETS plasma_applet_bookmarks  DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install( FILES plasma-applet-bookmarks.desktop  DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )
