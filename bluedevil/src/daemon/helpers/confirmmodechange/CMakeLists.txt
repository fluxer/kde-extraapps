set(confirmmodechange_SRCS
    main.cpp
    confirmmodechange.cpp
)

add_executable(bluedevil-confirmmodechange ${confirmmodechange_SRCS})

target_link_libraries(bluedevil-confirmmodechange
    ${KDE4_KIO_LIBS}
    ${KDE4_KDEUI_LIBS}
)

install(
    TARGETS bluedevil-confirmmodechange
    DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR}
)
