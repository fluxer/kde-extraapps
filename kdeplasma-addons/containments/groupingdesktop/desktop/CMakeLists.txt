
set(groupingdesktop_SRCS groupingdesktop.cpp)

kde4_add_plugin(plasma_containment_groupingdesktop ${groupingdesktop_SRCS})
target_link_libraries(plasma_containment_groupingdesktop KDE4::plasma plasma_groupingcontainment)

install(TARGETS plasma_containment_groupingdesktop DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-groupingdesktop.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})