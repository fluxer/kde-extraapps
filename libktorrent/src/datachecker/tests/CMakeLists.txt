set(datacheckertest_SRCS datacheckertest.cpp)

kde4_add_test(libktorrent-datacheckertest ${datacheckertest_SRCS})
target_link_libraries(libktorrent-datacheckertest ${QT_QTTEST_LIBRARY} testlib ktorrent)