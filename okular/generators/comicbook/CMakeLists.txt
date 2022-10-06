include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

########### next target ###############

set(okularGenerator_comicbook_PART_SRCS
    document.cpp
    generator_comicbook.cpp
    directory.cpp
    qnatsort.cpp
)

kde4_add_plugin(okularGenerator_comicbook ${okularGenerator_comicbook_PART_SRCS})

target_link_libraries(okularGenerator_comicbook
    ${KDE4_KIO_LIBS}
    ${KDE4_KARCHIVE_LIBS}
    okularcore
)

install(
    TARGETS okularGenerator_comicbook
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES
    libokularGenerator_comicbook.desktop
    okularComicbook.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
install(
    PROGRAMS okularApplication_comicbook.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

