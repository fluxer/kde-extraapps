include_directories( ${KDE4_INCLUDES} )

set(yakuake_SRCS
    main.cpp
    application.cpp
    mainwindow.cpp
    skin.cpp
    titlebar.cpp
    tabbar.cpp
    sessionstack.cpp
    session.cpp
    terminal.cpp
    splitter.cpp
    firstrundialog.cpp
    visualeventoverlay.cpp
    config/windowsettings.cpp
    config/appearancesettings.cpp
    config/skinlistdelegate.cpp
)


kde4_add_kcfg_files(yakuake_SRCS config/settings.kcfgc)

add_executable(yakuake ${yakuake_SRCS})

target_link_libraries(yakuake ${KDE4_KNOTIFYCONFIG_LIBS} ${KDE4_KPARTS_LIBS})

if(Q_WS_X11)
  target_link_libraries(yakuake ${X11_X11_LIB})
endif(Q_WS_X11)

install(TARGETS yakuake ${INSTALL_TARGETS_DEFAULT_ARGS})
