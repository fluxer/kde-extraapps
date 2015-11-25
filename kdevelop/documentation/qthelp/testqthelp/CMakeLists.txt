project(testqthelp)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
set(testqthelp_SRCS docwidget.cpp main.cpp)
qt4_wrap_ui(testqthelp_SRCS docwidget.ui)
qt4_automoc(${testqthelp_SRCS})
add_executable(testqthelp ${testqthelp_SRCS})
target_link_libraries(testqthelp ${QT_QTCORE_LIBRARIES} ${QT_QTGUI_LIBRARIES} ${QT_QTHELP_LIBRARIES} ${QT_QTWEBKIT_LIBRARIES})
