set(chunkmanagertest_SRCS chunkmanagertest.cpp)
kde4_add_test(libktorrent-chunkmanagertest ${chunkmanagertest_SRCS})
target_link_libraries(libktorrent-chunkmanagertest ${QT_QTTEST_LIBRARY} testlib ktorrent)

set(preallocationtest_SRCS preallocationtest.cpp)
kde4_add_test(libktorrent-preallocationtest ${preallocationtest_SRCS})
target_link_libraries(libktorrent-preallocationtest ${QT_QTTEST_LIBRARY} testlib ktorrent)