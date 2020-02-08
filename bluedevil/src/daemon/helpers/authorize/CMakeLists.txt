set(authorize_SRCS
    main.cpp
    authorize.cpp
)

add_executable(bluedevil-authorize ${authorize_SRCS})

target_link_libraries(bluedevil-authorize
                      ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS} bluedevil)

install(TARGETS bluedevil-authorize
        DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})
