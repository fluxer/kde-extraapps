include_directories(
   ${DJVULIBRE_INCLUDE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_BINARY_DIR}/okular
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=4712)

########### next target ###############

set(okularGenerator_djvu_SRCS
   generator_djvu.cpp
   kdjvu.cpp
)


kde4_add_plugin(okularGenerator_djvu ${okularGenerator_djvu_SRCS})

target_link_libraries(okularGenerator_djvu okularcore ${DJVULIBRE_LIBRARY} KDE4::kdeui ${QT_QTXML_LIBRARY})

install(TARGETS okularGenerator_djvu DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_djvu.desktop okularDjvu.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_djvu.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )


