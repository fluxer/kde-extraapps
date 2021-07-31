project(kompare)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include(GenerateExportHeader)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

add_subdirectory( libkomparediff2 )
add_subdirectory( interfaces )
add_subdirectory( libdialogpages )
add_subdirectory( komparenavtreepart )
add_subdirectory( komparepart )
add_subdirectory( pics )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/komparepart
    ${CMAKE_CURRENT_BINARY_DIR}/libdialogpages
    ${CMAKE_CURRENT_SOURCE_DIR}/libdialogpages
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/komparenavtreepart
    ${CMAKE_CURRENT_SOURCE_DIR}/komparepart
    ${CMAKE_CURRENT_BINARY_DIR}/libkomparediff2
    ${CMAKE_CURRENT_SOURCE_DIR}/libkomparediff2
)

########### next target ###############

set(kompare_SRCS
    main.cpp
    kompare_shell.cpp
    kompareurldialog.cpp
)

add_executable(kompare ${kompare_SRCS})

target_link_libraries(kompare
    kompareinterface
    komparediff2
    komparedialogpages
    ${KDE4_KTEXTEDITOR_LIBS}
)

install(TARGETS kompare ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(
    PROGRAMS kompare.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
install(
    FILES kompareui.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kompare
)
install(
    FILES
    komparenavigationpart.desktop
    kompareviewpart.desktop
    DESTINATION  ${KDE4_SERVICETYPES_INSTALL_DIR}
)
