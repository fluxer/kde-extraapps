
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# okular

set(okular_SRCS
   main.cpp
   okular_main.cpp
   shell.cpp
   shellutils.cpp
)

add_executable(okular ${okular_SRCS})

target_link_libraries(okular ${KDE4_KPARTS_LIBS})

install(TARGETS okular ${INSTALL_TARGETS_DEFAULT_ARGS})


# okular shell data files

install( PROGRAMS okular.desktop  DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR} )
install( FILES shell.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/okular )

