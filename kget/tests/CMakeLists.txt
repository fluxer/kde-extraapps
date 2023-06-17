include_directories(
   ../
)

set(kget_test_transfers_SRCS
    testkget.cpp
    testtransfers.cpp
)

qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.main.xml kget_interface)
qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.transfer.xml transfer_interface)
qt4_add_dbus_interface(kget_test_transfers_SRCS ../dbus/org.kde.kget.verifier.xml verifier_interface)

kde4_add_test(kget-test_transfers ${kget_test_transfers_SRCS})

target_link_libraries(kget-test_transfers ${QT_QTTEST_LIBRARY} KDE4::kio kgetcore)


#===========Verifier===========
kde4_add_test(kget-verifiertest verifiertest.cpp)

target_link_libraries(kget-verifiertest ${QT_QTTEST_LIBRARY} KDE4::kdecore kgetcore)


#===========Scheduler===========
kde4_add_test(kget-schedulertest schedulertest.cpp)

target_link_libraries(kget-schedulertest ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY} KDE4::kdecore kgetcore)

#===========FileDeleter===========
set(filedeletertest_SRCS
    filedeletertest.cpp
    ../core/filedeleter.cpp
)

kde4_add_test(kget-filedeletertest ${filedeletertest_SRCS})

target_link_libraries(kget-filedeletertest KDE4::kio ${QT_QTTEST_LIBRARY})
