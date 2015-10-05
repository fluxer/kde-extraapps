# project(context-similarartists)

include_directories(
    ../..
    ../../..
    ${CMAKE_SOURCE_DIR}/src
    )

set(similarArtists_SRCS
    SimilarArtist.cpp
    ArtistWidget.cpp
    SimilarArtistsApplet.cpp
    similarArtistsSettings.ui
)

kde4_add_plugin(amarok_context_applet_similarArtists ${similarArtists_SRCS})
target_link_libraries(amarok_context_applet_similarArtists
    amarokcore
    amaroklib
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KDEUI_LIBS}
)

install(TARGETS amarok_context_applet_similarArtists DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES amarok-context-applet-similarArtists.desktop DESTINATION ${SERVICES_INSTALL_DIR})
