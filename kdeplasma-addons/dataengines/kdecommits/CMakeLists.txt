project(plasma-kdecommitsengine)

set(kdecommits_engine_SRCS
    src/kdepresets.cpp
    src/kdecommitsengine.cpp
    src/kdecommitsservice.cpp
)
 
kde4_add_plugin(plasma_engine_kdecommits ${kdecommits_engine_SRCS})

target_link_libraries(plasma_engine_kdecommits
                      ${QT_QTNETWORK_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDE4_PLASMA_LIBS})
 
install(TARGETS plasma_engine_kdecommits
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-kdecommits.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES src/kdecommits.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
