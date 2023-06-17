project(plasma-notes)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

set(notes_SRCS notes.cpp textedit.cpp config.ui)

kde4_add_plugin(plasma_applet_notes ${notes_SRCS})
target_link_libraries(plasma_applet_notes
            KDE4::plasma KDE4::kio
            )
install(
    TARGETS plasma_applet_notes
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

install(
    FILES plasma-notes-default.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )

install(
    FILES notes.svgz
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
