kde4_add_test(clirarplugin-clirartest clirartest.cpp ../cliplugin.cpp)
target_link_libraries(clirarplugin-clirartest kerfuffle ${QT_QTTEST_LIBRARY} ${KDE4_KPARTS_LIBRARY})
