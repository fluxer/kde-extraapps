project(plasma-applet-kimpanel)

include_directories(${X11_X11_INCLUDE_PATH})

set(plasma_applet_kimpanel_SRCS
    kimpanel.cpp
    kimpanelinputpanel.cpp
    kimpanelinputpanelgraphics.cpp
    kimpanelsettings.cpp
    kimpanellabelgraphics.cpp
    kimpanelstatusbargraphics.cpp
    icongridlayout.cpp
    paintutils.cpp
    dummywidget.cpp
    config.ui
)

kde4_add_kcfg_files(plasma_applet_kimpanel_SRCS kimpanelconfig.kcfgc)
kde4_add_plugin(plasma_applet_kimpanel ${plasma_applet_kimpanel_SRCS})
target_link_libraries(plasma_applet_kimpanel
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${X11_X11_LIB}
                      )

install(TARGETS plasma_applet_kimpanel
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-kimpanel.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES kimpanelconfig.kcfg
         DESTINATION ${KDE4_KCFG_INSTALL_DIR})
