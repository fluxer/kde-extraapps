set(requestpin_SRCS
    main.cpp
    requestpin.cpp
    dialogWidget.ui
)

add_executable(bluedevil-requestpin ${requestpin_SRCS} ${requestpin_UI})

target_link_libraries(bluedevil-requestpin
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS})

install(TARGETS bluedevil-requestpin
        DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})
