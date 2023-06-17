set(arkpart_PART_SRCS
    part.cpp
    infopanel.cpp
    arkviewer.cpp
    archivemodel.cpp
    archiveview.cpp
    jobtracker.cpp
)

qt4_add_dbus_adaptor(arkpart_PART_SRCS dnddbusinterface.xml part.h Ark::Part)

kde4_add_plugin(arkpart ${arkpart_PART_SRCS})

target_link_libraries(arkpart kerfuffle KDE4::kfile KDE4::kparts KDE4::kdeui)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ark_part.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ark_part.desktop
)

install(TARGETS arkpart  DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/ark_part.desktop     DESTINATION ${KDE4_SERVICES_INSTALL_DIR} )
install( FILES ark_part.rc          DESTINATION ${KDE4_DATA_INSTALL_DIR}/ark )
