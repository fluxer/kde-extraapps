set(streamingchunkselectortest_SRCS streamingchunkselectortest.cpp)

kde4_add_test(libktorrent-streamingchunkselectortest ${streamingchunkselectortest_SRCS})
target_link_libraries(libktorrent-streamingchunkselectortest ${QT_QTTEST_LIBRARY} testlib ktorrent)