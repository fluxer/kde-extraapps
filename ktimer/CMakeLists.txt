project(ktimer)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.18.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(ktimer_SRCS main.cpp ktimer.cpp prefwidget.ui )

add_executable(ktimer ${ktimer_SRCS})

target_link_libraries(ktimer  ${KDE4_KIO_LIBS})

install(TARGETS ktimer  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS ktimer.desktop  DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

kde4_install_icons( ${KDE4_ICON_INSTALL_DIR}   )
