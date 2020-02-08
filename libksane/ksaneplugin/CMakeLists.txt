project(ksaneplugin)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../libksane
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
add_definitions(
    ${QT_QTDBUS_DEFINITIONS}
    -DQT_USE_FAST_CONCATENATION
    -DQT_USE_FAST_OPERATOR_PLUS
)
include_directories(${QDBUS_INCLUDE_DIRS})

########### target ###############
kde4_add_plugin(ksaneplugin sanedialog.cpp)

target_link_libraries(ksaneplugin ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ksane)

install(TARGETS ksaneplugin  DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )

########### install files ###############
install( FILES ksane_scan_service.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )

