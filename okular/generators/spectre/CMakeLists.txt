include_directories(
   ${LIBSPECTRE_INCLUDE_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}/../..
   ${CMAKE_CURRENT_BINARY_DIR}/../..
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=4711)

add_subdirectory(conf)

########### next target ###############

set(okularGenerator_ghostview_SRCS
    generator_ghostview.cpp
    gssettingswidget.ui
)

kde4_add_kcfg_files(okularGenerator_ghostview_SRCS conf/gssettings.kcfgc)

kde4_add_plugin(okularGenerator_ghostview ${okularGenerator_ghostview_SRCS})

target_link_libraries(okularGenerator_ghostview
    KDE4::kdeui
    ${QT_QTXML_LIBRARY}
    ${LIBSPECTRE_LIBRARY}
    okularcore
)

install(
    TARGETS okularGenerator_ghostview
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES
    libokularGenerator_ghostview.desktop
    okularGhostview.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
install(
    PROGRAMS okularApplication_ghostview.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
