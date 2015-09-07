set(highlightingtest_SRCS test_highlighting.cpp)

kde4_add_test(kdevplatform-highlightingtest ${highlightingtest_SRCS})
target_link_libraries(kdevplatform-highlightingtest ${KDE4_KDECORE_LIBS} ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY} 
    ${KDEVPLATFORM_TESTS_LIBRARIES} kdevplatformlanguage)
