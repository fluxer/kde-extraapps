include_directories(${CMAKE_CURRENT_BINARY_DIR}/..)

kde4_add_test(okular-shelltest shelltest.cpp ../shell/shellutils.cpp )
target_link_libraries(okular-shelltest KDE4::kdecore ${QT_QTTEST_LIBRARY} )

kde4_add_test(okular-parttest parttest.cpp )
target_link_libraries(okular-parttest KDE4::kdecore KDE4::kparts ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} okularpart okularcore )

kde4_add_test(okular-documenttest documenttest.cpp )
target_link_libraries(okular-documenttest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} okularcore )

kde4_add_test(okular-searchtest searchtest.cpp )
target_link_libraries(okular-searchtest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} okularcore )

kde4_add_test(okular-annotationstest annotationstest.cpp )
target_link_libraries(okular-annotationstest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} okularcore )

kde4_add_test(okular-editannotationcontentstest editannotationcontentstest.cpp testingutils.cpp)
target_link_libraries(okular-editannotationcontentstest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} okularcore )

kde4_add_test(okular-addremoveannotationtest addremoveannotationtest.cpp testingutils.cpp)
target_link_libraries(okular-addremoveannotationtest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} okularcore )

kde4_add_test(okular-translateannotationtest translateannotationtest.cpp testingutils.cpp)
target_link_libraries(okular-translateannotationtest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} okularcore )

kde4_add_test(okular-modifyannotationpropertiestest modifyannotationpropertiestest.cpp testingutils.cpp)
target_link_libraries(okular-modifyannotationpropertiestest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} okularcore )

# forms are not implemented
# kde4_add_test(okular-editformstest editformstest.cpp )
# target_link_libraries(okular-editformstest KDE4::kdecore ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTXML_LIBRARY} okularcore )

kde4_add_test(okular-mainshelltest mainshelltest.cpp ../shell/okular_main.cpp ../shell/shellutils.cpp ../shell/shell.cpp )
target_link_libraries(okular-mainshelltest KDE4::kparts ${QT_QTTEST_LIBRARY} okularpart okularcore )
