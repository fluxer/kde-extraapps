set(localwindowtest_SRCS localwindowtest.cpp)
kde4_add_test(libktorrent-localwindowtest ${localwindowtest_SRCS})
target_link_libraries(libktorrent-localwindowtest ${QT_QTTEST_LIBRARY} ktorrent)

set(remotewindowtest_SRCS remotewindowtest.cpp)
kde4_add_test(libktorrent-remotewindowtest ${remotewindowtest_SRCS})
target_link_libraries(libktorrent-remotewindowtest ${QT_QTTEST_LIBRARY} ktorrent)

set(connectiontest_SRCS connectiontest.cpp)
kde4_add_test(libktorrent-connectiontest ${connectiontest_SRCS})
target_link_libraries(libktorrent-connectiontest ${QT_QTTEST_LIBRARY} ktorrent)

set(sendtest_SRCS sendtest.cpp)
kde4_add_test(libktorrent-sendtest ${sendtest_SRCS})
target_link_libraries(libktorrent-sendtest ${QT_QTTEST_LIBRARY} ktorrent)

set(connecttest_SRCS connecttest.cpp)
kde4_add_test(libktorrent-connecttest ${connecttest_SRCS})
target_link_libraries(libktorrent-connecttest ${QT_QTTEST_LIBRARY} ktorrent)

set(transmittest_SRCS transmittest.cpp)
kde4_add_test(libktorrent-transmittest ${transmittest_SRCS})
target_link_libraries(libktorrent-transmittest ${QT_QTTEST_LIBRARY} ktorrent)

set(packetlosstest_SRCS packetlosstest.cpp)
kde4_add_test(libktorrent-packetlosstest ${packetlosstest_SRCS})
target_link_libraries(libktorrent-packetlosstest ${QT_QTTEST_LIBRARY} ktorrent)

set(fintest_SRCS fintest.cpp)
kde4_add_test(libktorrent-fintest ${fintest_SRCS})
target_link_libraries(libktorrent-fintest ${QT_QTTEST_LIBRARY} ktorrent)

set(sockettest_SRCS sockettest.cpp)
kde4_add_test(libktorrent-sockettest ${sockettest_SRCS})
target_link_libraries(libktorrent-sockettest ${QT_QTTEST_LIBRARY} ktorrent)

set(utppolltest_SRCS utppolltest.cpp)
kde4_add_test(libktorrent-utppolltest ${utppolltest_SRCS})
target_link_libraries(libktorrent-utppolltest ${QT_QTTEST_LIBRARY} ktorrent)

set(delaywindowtest_SRCS delaywindowtest.cpp)
kde4_add_test(libktorrent-delaywindowtest ${delaywindowtest_SRCS})
target_link_libraries(libktorrent-delaywindowtest ${QT_QTTEST_LIBRARY} ktorrent)

set(packetbuffertest_SRCS packetbuffertest.cpp)
kde4_add_test(libktorrent-packetbuffertest ${packetbuffertest_SRCS})
target_link_libraries(libktorrent-packetbuffertest ${QT_QTTEST_LIBRARY} ktorrent)