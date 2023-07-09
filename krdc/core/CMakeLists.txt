add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5010)

set(krdccore_SRCS
    remoteviewfactory.cpp
    remoteview.cpp
    hostpreferences.cpp
)

kde4_add_kcfg_files(krdccore_SRCS settings.kcfgc)

add_library(krdccore SHARED ${krdccore_SRCS})

target_link_libraries(krdccore
    KDE4::kdecore
    KDE4::kdeui
    KDE4::kpasswdstore
)

set_target_properties(krdccore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(krdccore)

install(
    TARGETS krdccore
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

# Install headers
set(krdccore_HDRS
    remoteviewfactory.h
    remoteview.h
    hostpreferences.h
)

install(
    FILES ${krdccore_HDRS}
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/krdc
)

install(
    FILES krdc_plugin.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)
