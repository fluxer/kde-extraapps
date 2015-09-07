include_directories(${LIBKTORRENT_SOURCE_DIR})

set(statsfiletest_SRCS statsfiletest.cpp)
kde4_add_test(libktorrent-statsfiletest ${statsfiletest_SRCS})
target_link_libraries(libktorrent-statsfiletest ${QT_QTTEST_LIBRARY} ktorrent)

set(torrentfilestreamtest_SRCS torrentfilestreamtest.cpp)
kde4_add_test(libktorrent-torrentfilestreamtest ${torrentfilestreamtest_SRCS})
target_link_libraries(libktorrent-torrentfilestreamtest ${QT_QTTEST_LIBRARY} testlib ktorrent)

set(torrentfilestreammultitest_SRCS torrentfilestreammultitest.cpp)
kde4_add_test(libktorrent-torrentfilestreammultitest ${torrentfilestreammultitest_SRCS})
target_link_libraries(libktorrent-torrentfilestreammultitest ${QT_QTTEST_LIBRARY} testlib ktorrent)