project(context-currenttrack)

set( wiki_SRCS WikipediaApplet.cpp wikipediaGeneralSettings.ui wikipediaLanguageSettings.ui)

include_directories(${Amarok_SOURCE_DIR}/src
                    ${Amarok_SOURCE_DIR}/src/context
                    ${Amarok_SOURCE_DIR}/src/network
                    )

kde4_add_plugin(amarok_context_applet_wikipedia ${wiki_SRCS})
target_link_libraries(amarok_context_applet_wikipedia
    amarokcore
    amaroklib
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEWEBKIT_LIBS}
    ${QT_QTWEBKIT_LIBRARY}
)

install(TARGETS amarok_context_applet_wikipedia DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES amarok-context-applet-wikipedia.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES amarok-wikipedia.svg amarok-wikipediaheader.svg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/ )
install(FILES WikipediaCustomStyle.css bullet.gif DESTINATION ${DATA_INSTALL_DIR}/amarok/data )
