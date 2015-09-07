add_definitions(-DTEST_DATA="${CMAKE_CURRENT_SOURCE_DIR}/configs/")

include_directories(${QT_INCLUDES} ${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${X11_Xrandr_INCLUDE_PATH})

macro(KSCREEN_TESTS)
    foreach(_testname ${ARGN})
        kde4_add_test(libkscreen-${_testname} ${_testname}.cpp)
        target_link_libraries(libkscreen-${_testname} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} kscreen)
    endforeach(_testname)
endmacro(KSCREEN_TESTS)


KSCREEN_TESTS(testscreenconfig)

if (ENABLE_XRANDR_TESTS)
    KSCREEN_TESTS(testxrandr)
endif()
