########### next target ###############

set(SUPPORTED_KARCHIVE_MIMETYPES "application/x-archive;application/zip;application/x-bzip;application/x-gzip;application/x-zip-compressed;application/x-tar;application/x-compressed-tar;application/x-bzip-compressed-tar;application/x-gzip-compressed-tar;")

set(kerfuffle_karchive_SRCS karchiveplugin.cpp)

kde4_add_plugin(kerfuffle_karchive ${kerfuffle_karchive_SRCS})

target_link_libraries(kerfuffle_karchive  ${KDE4_KDECORE_LIBS} kerfuffle )

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/kerfuffle_karchive.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle_karchive.desktop
)

########### install files ###############

install(TARGETS kerfuffle_karchive  DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )
install( FILES kerfuffle_karchive.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )

set(SUPPORTED_ARK_MIMETYPES "${SUPPORTED_ARK_MIMETYPES}${SUPPORTED_KARCHIVE_MIMETYPES}" PARENT_SCOPE)
