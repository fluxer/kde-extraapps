project(kfloppy)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

if(NOT UNIX)
  message(FATAL_ERROR "KFloppy is only supposed to be built on Unix platforms.")
endif(NOT UNIX)

set(kfloppy_SRCS main.cpp format.cpp floppy.cpp )

add_executable(kfloppy ${kfloppy_SRCS})

target_link_libraries(kfloppy ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kfloppy  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS KFloppy.desktop  DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

# The mimetypes used in this servicemenu (e.g. floppy_unmounted) don't exist anymore.
# This needs to be done differently.
#install( FILES floppy_format.desktop  DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus)

kde4_install_icons( ${KDE4_ICON_INSTALL_DIR}   )
