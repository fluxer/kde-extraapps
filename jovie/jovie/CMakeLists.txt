include_directories(${SPEECHD_INCLUDE_DIR})

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=2400)

########### next target ###############

set(jovie_SRCS
    main.cpp
    jovie.cpp
    speaker.cpp
    appdata.cpp
    ssmlconvert.cpp
    filtermgr.cpp
    talkermgr.cpp
    jovietrayicon.cpp
)

qt4_add_dbus_adaptor(jovie_SRCS
    ${KDE4_DBUS_INTERFACES_INSTALL_DIR}/org.kde.KSpeech.xml
    jovie.h
    Jovie
)

add_executable(jovie_bin ${jovie_SRCS})

set_target_properties(jovie_bin PROPERTIES OUTPUT_NAME jovie)

target_link_libraries(jovie_bin
    ${SPEECHD_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    kttsd
)

install(TARGETS jovie_bin ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(
    FILES SSMLtoPlainText.xsl
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/jovie/xslt
)

install(
    FILES
    jovie.desktop
    kttsd.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

install(
    PROGRAMS jovieapp.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

