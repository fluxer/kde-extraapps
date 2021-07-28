
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../libdiff2
    ${CMAKE_CURRENT_SOURCE_DIR}/../komparepart
    ${CMAKE_CURRENT_BINARY_DIR}/../libkomparediff2
    ${CMAKE_CURRENT_SOURCE_DIR}/../libkomparediff2
)


########### next target ###############

set(komparenavtreepart_PART_SRCS komparenavtreepart.cpp )


kde4_add_plugin(komparenavtreepart ${komparenavtreepart_PART_SRCS})



target_link_libraries(komparenavtreepart  komparediff2 ${KDE4_KPARTS_LIBS} )

install(TARGETS komparenavtreepart  DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES komparenavtreepart.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )

