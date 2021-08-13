project(kcolorchooser)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(kcolorchooser_SRCS kcolorchooser.cpp )

add_executable(kcolorchooser ${kcolorchooser_SRCS})

target_link_libraries(kcolorchooser ${KDE4_KDEUI_LIBS})

install(TARGETS kcolorchooser  ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( PROGRAMS kcolorchooser.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )

kde4_install_icons( ${KDE4_ICON_INSTALL_DIR}   )

