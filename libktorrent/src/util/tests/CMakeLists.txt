set(circularbuffertest_SRCS circularbuffertest.cpp)
kde4_add_test(libktorrent-circularbuffertest ${circularbuffertest_SRCS})
target_link_libraries(libktorrent-circularbuffertest ${QT_QTTEST_LIBRARY} ktorrent)

set(signalcatchertest_SRCS signalcatchertest.cpp)
kde4_add_test(libktorrent-signalcatchertest ${signalcatchertest_SRCS})
target_link_libraries(libktorrent-signalcatchertest ${QT_QTTEST_LIBRARY} ktorrent)

set(resourcemanagertest_SRCS resourcemanagertest.cpp)
kde4_add_test(libktorrent-resourcemanagertest ${resourcemanagertest_SRCS})
target_link_libraries(libktorrent-resourcemanagertest ${QT_QTTEST_LIBRARY} ktorrent)

set(fileopstest_SRCS fileopstest.cpp)
kde4_add_test(libktorrent-fileopstest ${fileopstest_SRCS})
target_link_libraries(libktorrent-fileopstest ${KDE4_SOLID_LIBS} ${QT_QTTEST_LIBRARY} ktorrent)

set(bufferpooltest_SRCS bufferpooltest.cpp)
kde4_add_test(libktorrent-bufferpooltest ${bufferpooltest_SRCS})
target_link_libraries(libktorrent-bufferpooltest ${KDE4_SOLID_LIBS} ${QT_QTTEST_LIBRARY} ktorrent)