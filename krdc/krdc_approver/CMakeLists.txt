include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (krdc_rfb_approver_SRCS
    main.cpp
    approvermanager.cpp
    approver.cpp
)

add_executable(krdc_rfb_approver ${krdc_rfb_approver_SRCS})
target_link_libraries(krdc_rfb_approver ${KDE4_KDEUI_LIBS}
                                    ${TELEPATHY_QT4_LIBRARIES}
)

install(TARGETS krdc_rfb_approver ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES krdc_rfb_approver.notifyrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/krdc_rfb_approver)

configure_file(org.freedesktop.Telepathy.Client.krdc_rfb_approver.service.in
                ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.krdc_rfb_approver.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.krdc_rfb_approver.service
        DESTINATION ${KDE4_DBUS_SERVICES_INSTALL_DIR})

# FIXME any better macro for ${XDG_DATA_DIRS} ?
install(FILES krdc_rfb_approver.client DESTINATION ${KDE4_SHARE_INSTALL_PREFIX}/telepathy/clients/)
