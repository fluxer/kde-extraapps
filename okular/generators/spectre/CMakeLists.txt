add_subdirectory( conf ) 

include_directories(
   ${LIBSPECTRE_INCLUDE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}/../..
)

########### next target ###############

set(okularGenerator_ghostview_SRCS
   generator_ghostview.cpp
   rendererthread.cpp
   gssettingswidget.ui
)

kde4_add_kcfg_files(okularGenerator_ghostview_SRCS conf/gssettings.kcfgc )


kde4_add_plugin(okularGenerator_ghostview ${okularGenerator_ghostview_SRCS})

target_link_libraries(okularGenerator_ghostview okularcore ${LIBSPECTRE_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})

install(TARGETS okularGenerator_ghostview DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_ghostview.desktop okularGhostview.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_ghostview.desktop DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )

kde4_install_icons(${KDE4_DATA_INSTALL_DIR}/okular/icons)
