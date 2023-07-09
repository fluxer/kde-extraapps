########### next target ###############

set(kerfuffle_SRCS
    archive.cpp
    archiveinterface.cpp
    jobs.cpp
    extractiondialog.cpp
    adddialog.cpp
    queries.cpp
    addtoarchive.cpp
    extractiondialog.ui
    adddialog.ui
)

kde4_add_kcfg_files(kerfuffle_SRCS settings.kcfgc)

add_library(kerfuffle SHARED ${kerfuffle_SRCS})
target_link_libraries(kerfuffle
    KDE4::kfile
    KDE4::kparts
    KDE4::kpty
)

set_target_properties(kerfuffle PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(kerfuffle)

install(
    TARGETS kerfuffle
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

install(
    FILES kerfufflePlugin.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

if (ENABLE_TESTING)
    add_subdirectory(tests)
endif ()
