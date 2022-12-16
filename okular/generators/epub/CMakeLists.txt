include_directories(
    ${EPUB_INCLUDE_DIR}
)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=4713)

add_subdirectory(data)

########### next target ###############

set(okularGenerator_epub_PART_SRCS
    converter.cpp
    epubdocument.cpp
    generator_epub.cpp
)

kde4_add_plugin(okularGenerator_epub ${okularGenerator_epub_PART_SRCS})

target_link_libraries(okularGenerator_epub okularcore ${EPUB_LIBRARIES} ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY}  ${QT_QTXML_LIBRARY})

install(TARGETS okularGenerator_epub DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_epub.desktop okularEPub.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_epub.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )
