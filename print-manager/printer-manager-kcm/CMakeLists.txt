set(kcm_print_SRCS
    PrinterDelegate.cpp
    PrinterDescription.cpp
    PrintKCM.cpp
    PrintKCM.ui
    PrinterDescription.ui
)

kde4_add_plugin(kcm_printer_manager ${kcm_print_SRCS})

target_link_libraries(kcm_printer_manager
    ${KDE4_KDEUI_LIBS}
    kcupslib
)

install(TARGETS kcm_printer_manager DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES kcm_printer_manager.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
