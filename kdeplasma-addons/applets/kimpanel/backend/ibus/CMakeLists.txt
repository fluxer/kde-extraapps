include_directories(${IBUS_INCLUDE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

if (${IBUS_VERSION} VERSION_LESS "1.5.0")
    set(kimpanel_ibus_panel_SRCS ibus14/panel.c ibus14/main.c)
    add_executable(kimpanel-ibus-panel ${kimpanel_ibus_panel_SRCS})
    target_link_libraries(kimpanel-ibus-panel ${IBUS_LIBRARIES})
else()
    set(kimpanel_ibus_panel_SRCS
        ibus15/xkblayoutmanager.cpp
        ibus15/gtkaccelparse_p.c
        ibus15/gdkkeynames_p.c
        ibus15/app.cpp
        ibus15/enginemanager.cpp
        ibus15/main.cpp
        ibus15/panel.cpp
        ibus15/propertymanager.cpp)
    add_definitions(-DQT_NO_KEYWORDS)

    add_executable(kimpanel-ibus-panel ${kimpanel_ibus_panel_SRCS})
    target_link_libraries(kimpanel-ibus-panel ${IBUS_LIBRARIES}
        ${QT_QTCORE_LIBRARY}
        ${QT_QTDBUS_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${X11_X11_LIB}
    )
endif()

install(TARGETS kimpanel-ibus-panel DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kimpanel.xml.in ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kimpanel.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ibus/component)

