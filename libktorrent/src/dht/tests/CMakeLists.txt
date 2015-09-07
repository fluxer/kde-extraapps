set(rpcmsgtest_SRCS rpcmsgtest.cpp)
kde4_add_test(libktorrent-rpcmsgtest ${rpcmsgtest_SRCS})
target_link_libraries(libktorrent-rpcmsgtest ${QT_QTTEST_LIBRARY} testlib ktorrent)

set(keytest_SRCS keytest.cpp)
kde4_add_test(libktorrent-keytest ${keytest_SRCS})
target_link_libraries(libktorrent-keytest ${QT_QTTEST_LIBRARY} testlib ktorrent)