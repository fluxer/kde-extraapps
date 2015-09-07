include_directories(
   ../
)

add_definitions(-DDO_KGET_TEST)
set(kget_test_transfers_SRCS
    testkget.cpp
    testtransfers.cpp
)

qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.main.xml kget_interface)
qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.transfer.xml transfer_interface)
qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.verifier.xml verifier_interface)

kde4_add_test(kget_test_transfers ${kget_test_transfers_SRCS})

target_link_libraries(kget_test_transfers ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kgetcore)


#===========Verifier===========
if (QCA2_FOUND)
    kde4_add_test(kget-verifiertest verifiertest.cpp)

    target_link_libraries(kget-verifiertest ${QT_QTTEST_LIBRARY} ${KDE4_KDECORE_LIBS} ${QCA2_LIBRARIES} kgetcore)
endif (QCA2_FOUND)


#===========Scheduler===========
kde4_add_test(kget-schedulertest schedulertest.cpp)

target_link_libraries(kget-schedulertest ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} kgetcore)


#===========Metalinker===========
set(metalinktest_SRCS
    metalinktest.cpp
    ../ui/metalinkcreator/metalinker.cpp
)

kde4_add_test(kget-metalinktest ${metalinktest_SRCS})

target_link_libraries(kget-metalinktest ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})

#===========FileDeleter===========
set(filedeletertest_SRCS
    filedeletertest.cpp
    ../core/filedeleter.cpp
)

kde4_add_test(kget-filedeletertest ${filedeletertest_SRCS})

target_link_libraries(kget-filedeletertest ${KDE4_KIO_LIBS} ${QT_QTTEST_LIBRARY})
