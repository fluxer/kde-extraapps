
set(griddesktop_SRCS griddesktop.cpp)

kde4_add_plugin(plasma_containment_griddesktop ${griddesktop_SRCS})
target_link_libraries(plasma_containment_griddesktop KDE4::plasma plasma_groupingcontainment)

install(TARGETS plasma_containment_griddesktop DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-griddesktop.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})