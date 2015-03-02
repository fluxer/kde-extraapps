add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")

include_directories(${QT_INCLUDES} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${X11_Xrandr_INCLUDE_PATH})

qt4_generate_moc(testscreenconfig.cpp ${CMAKE_CURRENT_BINARY_DIR}/testscreenconfig.moc)
kde4_add_unit_test(testscreenconfig testscreenconfig.cpp)
target_link_libraries(testscreenconfig ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kscreen)

if (ENABLE_XRANDR_TESTS)
    qt4_generate_moc(testxrandr.cpp ${CMAKE_CURRENT_BINARY_DIR}/testxrandr.moc)
    kde4_add_unit_test(testxrandr testxrandr.cpp)
    target_link_libraries(testxrandr ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kscreen)
endif()
