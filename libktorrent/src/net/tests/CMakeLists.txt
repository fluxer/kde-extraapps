set(polltest_SRCS polltest.cpp)
kde4_add_test(libktorrent-polltest ${polltest_SRCS})
target_link_libraries(libktorrent-polltest ${QT_QTTEST_LIBRARY} ktorrent)

set(wakeuppipetest_SRCS wakeuppipetest.cpp)
kde4_add_test(libktorrent-wakeuppipetest ${wakeuppipetest_SRCS})
target_link_libraries(libktorrent-wakeuppipetest ${QT_QTTEST_LIBRARY} ktorrent)