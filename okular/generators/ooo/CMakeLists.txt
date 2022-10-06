include_directories(
   ${CMAKE_BINARY_DIR}/okular
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

if ( OPENSSL_FOUND )
   include_directories( ${OPENSSL_INCLUDE_DIR} )
   add_definitions( -DHAVE_OPENSSL )
endif( OPENSSL_FOUND )

########### next target ###############

set(okularGenerator_ooo_PART_SRCS
    converter.cpp
    document.cpp
    formatproperty.cpp
    generator_ooo.cpp
    manifest.cpp
    styleinformation.cpp
    styleparser.cpp
)


kde4_add_plugin(okularGenerator_ooo ${okularGenerator_ooo_PART_SRCS})

target_link_libraries(okularGenerator_ooo
    ${KDE4_KIO_LIBS}
    ${KDE4_KARCHIVE_LIBS}
    ${MATH_LIB}
    ${QT_QTXML_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    okularcore
)
if (OPENSSL_FOUND)
    target_link_libraries(okularGenerator_ooo okularcore ${OPENSSL_LIBRARIES})
endif (OPENSSL_FOUND)

install(TARGETS okularGenerator_ooo DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_ooo.desktop okularOoo.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_ooo.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )

