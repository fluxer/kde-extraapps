find_package(KDE4Internal REQUIRED)

set(addprinter_SRCS
    main.cpp
    AddPrinter.cpp
    DevicesModel.cpp
    GenericPage.cpp
    PageDestinations.cpp
    ChooseLpd.cpp
    ChooseSamba.cpp
    ChooseSerial.cpp
    ChooseSocket.cpp
    ChooseUri.cpp
    PageChoosePrinters.cpp
    PageChoosePPD.cpp
    PageAddPrinter.cpp
    AddPrinterAssistant.cpp
    PageDestinations.ui
    ChooseLpd.ui
    ChooseSamba.ui
    ChooseSerial.ui
    ChooseSocket.ui
    ChooseUri.ui
    PageChoosePrinters.ui
    PageChoosePPD.ui
    PageAddPrinter.ui
)

add_executable(kde-add-printer
    ${addprinter_SRCS}
)

target_link_libraries(kde-add-printer
    ${KDE4_KDEUI_LIBS}
    ${QT_QTNETWORK_LIBRARY}
    ${KDE4_KIO_LIBS}
    kcupslib
)

install(TARGETS kde-add-printer DESTINATION ${KDE4_BIN_INSTALL_DIR})
