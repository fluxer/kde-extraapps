########### next target ###############

set(kerfuffle_SRCS
    archive.cpp
    archiveinterface.cpp
    jobs.cpp
    extractiondialog.cpp
    adddialog.cpp
    queries.cpp
    addtoarchive.cpp
    cliinterface.cpp
    extractiondialog.ui
    adddialog.ui
)

kde4_add_kcfg_files(kerfuffle_SRCS settings.kcfgc)

add_library(kerfuffle SHARED ${kerfuffle_SRCS})

target_link_libraries(kerfuffle
    ${KDE4_KFILE_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_KPTY_LIBS}
)

set_target_properties(kerfuffle PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS kerfuffle ${INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES kerfufflePlugin.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)
install(
    FILES ark.kcfg
    DESTINATION ${KDE4_KCFG_INSTALL_DIR}
)

if (ENABLE_TESTING)
    add_subdirectory(tests)
endif ()
