project(context-currenttrack)

set(lyrics_SRCS
    LyricsApplet.cpp
    LyricsBrowser.cpp
    LyricsSuggestionsListWidget.cpp
    lyricsSettings.ui
)

include_directories( ../..
                     ../../.. )

kde4_add_plugin(amarok_context_applet_lyrics ${lyrics_SRCS})
target_link_libraries(amarok_context_applet_lyrics
    amarokcore
    amaroklib
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS amarok_context_applet_lyrics DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES amarok-context-applet-lyrics.desktop DESTINATION ${SERVICES_INSTALL_DIR})
