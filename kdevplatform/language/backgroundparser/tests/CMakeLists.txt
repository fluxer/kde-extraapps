set(backgroundparsertest_SRCS test_backgroundparser.cpp testlanguagesupport.cpp testparsejob.cpp)

kde4_add_test(kdevplatform-backgroundparsertest ${backgroundparsertest_SRCS})
target_link_libraries(kdevplatform-backgroundparsertest
    ${KDE4_KDECORE_LIBS}
    ${KDE4_TEXTEDITOR_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDE4_THREADWEAVER_LIBS}
    kdevplatformlanguage)
