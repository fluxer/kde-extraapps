########### next target ###############

set(akregatorinterfaces_LIB_SRCS
    command.cpp
    feedlistmanagementinterface.cpp
    plugin.cpp
    storagefactoryregistry.cpp
)

kde4_add_kcfg_files(akregatorinterfaces_LIB_SRCS akregatorconfig.kcfgc)

add_library(akregatorinterfaces ${LIBRARY_TYPE} ${akregatorinterfaces_LIB_SRCS})

target_link_libraries(akregatorinterfaces
    ${KDE4_KDEUI_LIBS}
    syndication
)

set_target_properties(akregatorinterfaces PROPERTIES
    VERSION ${GENERIC_LIB_VERSION} 
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(TARGETS akregatorinterfaces ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES akregator.kcfg DESTINATION ${KCFG_INSTALL_DIR})

