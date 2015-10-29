add_subdirectory(plasma)

########### next target ###############

find_package(SharedMimeInfo REQUIRED)

set(kdevelop_SRCS
    main.cpp
    kdevideextension.cpp
    splash.cpp
    welcomepage/sessionsmodel.cpp
    welcomepage/welcomepageview.cpp
    welcomepage/uihelper.cpp
)
add_definitions(-DWITH_WELCOMEPAGE)
qt4_add_resources(kdevelop_SRCS welcomepage/welcomepage.qrc)

add_executable(kdevelop ${kdevelop_SRCS} )

target_link_libraries(kdevelop
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_SHELL_LIBRARIES}
    ${KDE4_KIO_LIBS}
    ${KDE4_KCMUTILS_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KNOTIFYCONFIG_LIBS}
    ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_THREADWEAVER_LIBS}

    ${QT_QTDECLARATIVE_LIBRARIES}
    ${KDE4_KDECLARATIVE_LIBS}
)

install(TARGETS kdevelop ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES kdevelop! DESTINATION bin PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)

########### install files ###############

install( PROGRAMS kdevelop.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( PROGRAMS kdevelop_ps.desktop  DESTINATION  ${XDG_APPS_INSTALL_DIR} )
install( FILES kdeveloprc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kdevelopui.rc katepartui.rc kdevelop.notifyrc DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
install( FILES kdevelop.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
install( FILES splash.qml DESTINATION ${DATA_INSTALL_DIR}/kdevelop )
update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
