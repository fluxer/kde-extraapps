include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_BINARY_DIR}/okular
   ${CMAKE_CURRENT_SOURCE_DIR}/md4c
)

########### next target ###############

set(okularGenerator_markdown_SRCS
   generator_md.cpp
   converter_md.cpp
   document_md.cpp
   md4c/md4c.c
   md4c/md4c-html.c
   md4c/entity.c
)

kde4_add_plugin(okularGenerator_md ${okularGenerator_markdown_SRCS})

target_link_libraries(okularGenerator_md okularcore ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS okularGenerator_md DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES libokularGenerator_md.desktop okularMarkdown.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( PROGRAMS okularApplication_md.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )
