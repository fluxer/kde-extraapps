project(plasma-fifteenPuzzle)

add_subdirectory(icons)
include_directories( ${KDE4_INCLUDES} )

set(fifteenPuzzle_SRCS src/fifteenPuzzle.cpp src/fifteen.cpp src/piece.cpp src/fifteenPuzzleConfig.ui )
kde4_add_plugin(plasma_applet_fifteenPuzzle ${fifteenPuzzle_SRCS})

target_link_libraries(plasma_applet_fifteenPuzzle ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})


install(TARGETS plasma_applet_fifteenPuzzle DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-fifteenPuzzle.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES images/blanksquare.svg DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/fifteenPuzzle/)
