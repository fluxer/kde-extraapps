set(wizard_SRCS
    main.cpp
    bluewizard.cpp
    wizardagent.cpp

    pages/discoverpage.cpp
    pages/nopairing.cpp
    pages/legacypairing.cpp
    pages/legacypairingdatabase.cpp
    pages/keyboardpairing.cpp
    pages/ssppairing.cpp
    pages/fail.cpp
    pages/success.cpp

    pages/discover.ui
    pages/nopairing.ui
    pages/legacypairing.ui
    pages/keyboardpairing.ui
    pages/ssppairing.ui
    pages/fail.ui
    pages/success.ui
)

add_executable(bluedevil-wizard ${wizard_SRCS})

target_link_libraries(bluedevil-wizard
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
    bluedevil
)

install(TARGETS bluedevil-wizard ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES bluedevil-wizard.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES pin-code-database.xml DESTINATION ${KDE4_DATA_INSTALL_DIR}/bluedevilwizard)
