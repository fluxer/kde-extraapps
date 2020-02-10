project(svgpart)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(svgpart_SRCS
    svgpart.cpp
)

kde4_add_plugin(svgpart ${svgpart_SRCS})

target_link_libraries(svgpart ${KDE4_KPARTS_LIBS})

install(TARGETS svgpart DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES svgpart.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES svgpart.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/svgpart)
