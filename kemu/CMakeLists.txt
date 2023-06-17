project(kemu)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5002)

find_program(QEMU_IMG_AND_QEMU_SYSTEM_EXECUTABLE qemu-img qemu-system-i386)
add_feature_info(qemu_img_and_qemu_system
    QEMU_IMG_AND_QEMU_SYSTEM_EXECUTABLE
    "Virtual machine management"
)

set(kemu_sources
    main.cpp
    kemumainwindow.cpp
)

add_executable(kemu ${kemu_sources})
target_link_libraries(kemu
    KDE4::kdeui
    KDE4::kfile
    ${QT_QTDBUS_LIBRARY}
)

install(TARGETS kemu DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES kemuui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kemu)
install(PROGRAMS kemu.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

add_subdirectory(kded)
add_subdirectory(krunner)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
