find_package(KDE4Internal REQUIRED)

set(queuemanager_SRCS
    main.cpp
    PrintQueue.cpp
    PrintQueueUi.cpp
    PrintQueueUi.ui
)

add_executable(kde-print-queue
    ${queuemanager_SRCS}
)

target_link_libraries(kde-print-queue
    ${KDE4_KDEUI_LIBS}
    kcupslib
)

install(TARGETS kde-print-queue DESTINATION ${KDE4_BIN_INSTALL_DIR})
