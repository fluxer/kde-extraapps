set(superseedtest_SRCS superseedtest.cpp)
kde4_add_test(libktorrent-superseedtest ${superseedtest_SRCS})
target_link_libraries(libktorrent-superseedtest ${QT_QTTEST_LIBRARY} ktorrent)

set(packetreadertest_SRCS packetreadertest.cpp)
kde4_add_test(libktorrent-packetreadertest ${packetreadertest_SRCS})
target_link_libraries(libktorrent-packetreadertest ${QT_QTTEST_LIBRARY} ktorrent)

set(connectionlimittest_SRCS connectionlimittest.cpp)
kde4_add_test(libktorrent-connectionlimittest ${connectionlimittest_SRCS})
target_link_libraries(libktorrent-connectionlimittest ${QT_QTTEST_LIBRARY} ktorrent)