include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ..
)

set(interactiveDiffTest_SRCS
    interactivedifftest.cpp
    ../perforceparser.cpp
    ../cvsdiffparser.cpp
    ../difference.cpp
    ../diffhunk.cpp
    ../diffmodel.cpp
    ../diffmodellist.cpp
    ../diffparser.cpp
    ../komparemodellist.cpp
    ../kompareprocess.cpp
    ../parser.cpp
    ../parserbase.cpp
    ../stringlistpair.cpp
)

kde4_add_test(kompare-interactiveDiffTest ${interactiveDiffTest_SRCS})
target_link_libraries(kompare-interactiveDiffTest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KPARTS_LIBS}
)

set(levenshteinTest_SRCS
    levenshteintest.cpp
    ../difference.cpp
    ../stringlistpair.cpp
)

kde4_add_test(kompare-levenshteintest ${levenshteinTest_SRCS})
target_link_libraries(kompare-levenshteintest
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)
