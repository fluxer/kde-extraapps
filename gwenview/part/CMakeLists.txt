include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${EXIV2_INCLUDE_DIR}
    )

set(gvpart_SRCS
    gvbrowserextension.cpp
    gvpart.cpp
    )

kde4_add_plugin(gvpart ${gvpart_SRCS})

target_link_libraries(gvpart ${KDE4_KPARTS_LIBS} gwenviewlib)

install(TARGETS gvpart DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES gvpart.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES gvpart.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/gvpart)
