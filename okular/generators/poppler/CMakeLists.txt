add_subdirectory( conf )

include_directories(
   ${CMAKE_BINARY_DIR}/okular
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${POPPLER_QT4_INCLUDE_DIR}
)

########### next target ###############

# for syntex, properties already set in the parent directory
find_package(ZLIB REQUIRED)

include_directories(
  synctex
  ${ZLIB_INCLUDE_DIR}
)

set(okularGenerator_poppler_PART_SRCS
   generator_pdf.cpp
   formfields.cpp
   annots.cpp
   synctex/synctex_parser.c
   synctex/synctex_parser_utils.c
)

kde4_add_kcfg_files(okularGenerator_poppler_PART_SRCS conf/pdfsettings.kcfgc )

kde4_add_plugin(okularGenerator_poppler ${okularGenerator_poppler_PART_SRCS})

target_link_libraries(okularGenerator_poppler
    ${KDE4_KDEUI_LIBS}
    ${QT_QTXML_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${POPPLER_QT4_LIBRARIES}
    okularcore
)

install(TARGETS okularGenerator_poppler DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

########### install files ###############

install(
    FILES libokularGenerator_poppler.desktop okularPoppler.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

install(
    PROGRAMS okularApplication_pdf.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

