include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

set(gvpart_SRCS
    gvpart.cpp
)

kde4_add_plugin(gvpart ${gvpart_SRCS})

target_link_libraries(gvpart KDE4::kparts gwenviewlib)

install(
    TARGETS gvpart
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
install(
    FILES gvpart.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
install(
    FILES gvpart.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/gvpart
)
