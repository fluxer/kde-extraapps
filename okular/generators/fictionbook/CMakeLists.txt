include_directories(
   ${CMAKE_BINARY_DIR}/okular
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
)


########### next target ###############

set(okularGenerator_fb_PART_SRCS
  converter.cpp
  document.cpp
  generator_fb.cpp
)


kde4_add_plugin(okularGenerator_fb ${okularGenerator_fb_PART_SRCS})

target_link_libraries(okularGenerator_fb okularcore KDE4::kio KDE4::karchive ${QT_QTXML_LIBRARY})

install(TARGETS okularGenerator_fb DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_fb.desktop okularFb.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_fb.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )

kde4_install_icons(${KDE4_DATA_INSTALL_DIR}/okular/icons)
