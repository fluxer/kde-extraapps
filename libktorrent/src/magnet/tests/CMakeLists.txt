set(magnetlinktest_SRCS magnetlinktest.cpp)

kde4_add_test(libktorrent-magnetlinktest ${magnetlinktest_SRCS})
target_link_libraries(libktorrent-magnetlinktest ${QT_QTTEST_LIBRARY} ktorrent)