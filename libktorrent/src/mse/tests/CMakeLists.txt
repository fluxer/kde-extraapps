set(rc4encryptortest_SRCS rc4encryptortest.cpp)
kde4_add_test(libktorrent-rc4encryptortest ${rc4encryptortest_SRCS})
target_link_libraries(libktorrent-rc4encryptortest ${QT_QTTEST_LIBRARY} ktorrent)