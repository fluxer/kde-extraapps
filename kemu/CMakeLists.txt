project(kemu)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDE4 4.23.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5002)

find_program(QEMU_EXECUTABLE NAMES qemu-img qemu-system-i386)
add_feature_info(kemu_qemu
    QEMU_EXECUTABLE
    "Virtual machine management"
)

set(kemu_sources
    main.cpp
    kemumainwindow.cpp
)

add_executable(kemu ${kemu_sources})
target_link_libraries(kemu
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${QT_QTDBUS_LIBRARY}
)

install(TARGETS kemu DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES kemuui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kemu)
install(PROGRAMS kemu.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

add_subdirectory(kded)
add_subdirectory(krunner)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
