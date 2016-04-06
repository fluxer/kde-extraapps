project(kcharselect)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.18.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

########### next target ###############

set(kcharselect_SRCS kcharselectdia.cc main.cc )

add_executable(kcharselect ${kcharselect_SRCS})

target_link_libraries(kcharselect  ${KDE4_KDEUI_LIBS} )

install(TARGETS kcharselect  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS KCharSelect.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kcharselectui.rc  DESTINATION ${DATA_INSTALL_DIR}/kcharselect)

kde4_install_icons( ${ICON_INSTALL_DIR}   )
