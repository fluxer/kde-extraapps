project(kemu)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(kemu_sources
    main.cpp
    kemumainwindow.cpp
)

add_executable(kemu ${kemu_sources})
target_link_libraries(kemu ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS})

install(TARGETS kemu DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES kemuui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kemu)
install(PROGRAMS kemu.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
