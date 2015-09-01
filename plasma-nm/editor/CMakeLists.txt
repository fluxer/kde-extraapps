include_directories(${PROJECT_SOURCE_DIR}/libs/editor
                    ${PROJECT_SOURCE_DIR}/libs/editor/widgets
                    ${PROJECT_SOURCE_DIR}/libs/models)

set(connectioneditor_SRCS
    main.cpp
    connectioneditor.cpp
    connectioneditor.ui
)

add_executable(kde-nm-connection-editor ${connectioneditor_SRCS})

target_link_libraries(kde-nm-connection-editor
    plasmanetworkmanagement-internal
    plasmanetworkmanagement-editor
    ${NETWORKMANAGERQT_LDFLAGS}
    ${MODEMMANAGERQT_LDFLAGS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

install(TARGETS kde-nm-connection-editor DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS kde-nm-connection-editor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES kde-nm-connection-editorui.rc  DESTINATION ${DATA_INSTALL_DIR}/kde-nm-connection-editor)
