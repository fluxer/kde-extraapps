include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
)


########### next target ###############

set(okularGenerator_xps_SRCS
   generator_xps.cpp
)


kde4_add_plugin(okularGenerator_xps ${okularGenerator_xps_SRCS})

target_link_libraries(okularGenerator_xps okularcore ${KDE4_KIO_LIBS} ${KDE4_KARCHIVE_LIBS} ${QT_QTXML_LIBRARY})

install(TARGETS okularGenerator_xps DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_xps.desktop okularXps.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_xps.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )

