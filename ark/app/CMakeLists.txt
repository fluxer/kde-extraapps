add_subdirectory(icons)

set(ark_SRCS
    batchextract.cpp
    main.cpp
    mainwindow.cpp
)

add_executable(ark ${ark_SRCS})

target_link_libraries(ark
    ${KDE4_KFILE_LIBS}
    ${KDE4_KPARTS_LIBS}
    kerfuffle
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ark.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ark.desktop
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ark_dndextract.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ark_dndextract.desktop
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ark_servicemenu.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ark_servicemenu.desktop
)

install(TARGETS ark ${INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES ark_addtoservicemenu.desktop ${CMAKE_CURRENT_BINARY_DIR}/ark_servicemenu.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus
)

install(
    PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ark.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

install(
    FILES arkui.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/ark
)

########### konqueror ark_extract_here plugin ###############

if (LibKonq_FOUND)
    include_directories(${LIBKONQ_INCLUDE_DIR})

    set(ark_dndextract_SRCS batchextract.cpp extractHereDndPlugin.cpp)
    kde4_add_plugin(ark_dndextract ${ark_dndextract_SRCS})

    target_link_libraries(ark_dndextract
        kerfuffle
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KPARTS_LIBS}
        ${KDE4_KFILE_LIBS}
        ${LIBKONQ_LIBRARY}
    )

    install(
        TARGETS ark_dndextract
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/ark_dndextract.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

