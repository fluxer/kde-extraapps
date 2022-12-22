########### next target ###############
set(SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES "application/x-archive;application/x-deb;application/x-cd-image;application/x-bcpio;application/x-cpio;application/x-cpio-compressed;application/x-sv4cpio;application/x-sv4crc;application/x-rpm;application/x-source-rpm;application/vnd.ms-cab-compressed;application/x-servicepack;")
set(SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES "application/x-xar;application/x-tar;application/x-compressed-tar;application/x-bzip-compressed-tar;application/x-gzip-compressed-tar;application/x-tarz;application/x-xz-compressed-tar;application/x-lzma-compressed-tar;application/x-java-archive;application/zip;application/x-7z-compressed;application/x-compress;application/x-lzip;")

if("${LibArchive_VERSION}" VERSION_GREATER "3.0.4")
    set(SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES "${SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES}application/x-lzop;application/x-lrzip;")
    set(SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES "${SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES}application/x-lzop;application/x-lrzip;")
endif()
if("${LibArchive_VERSION}" VERSION_GREATER "3.1.2")
    set(SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES "${SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES}application/x-lz4;application/x-lz4-compressed-tar;")
    set(SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES "${SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES}application/x-lz4;application/x-lz4-compressed-tar;")
endif()
if("${LibArchive_VERSION}" VERSION_GREATER "3.3.2")
    set(SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES "${SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES}application/zstd;application/x-zstd;application/x-zstd-compressed-tar;")
    set(SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES "${SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES}application/zstd;application/x-zstd;application/x-zstd-compressed-tar;")
endif()
if("${LibArchive_VERSION}" VERSION_GREATER "3.3.3")
    set(SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES "${SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES}application/vnd.rar;")
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/kerfuffle_libarchive_readonly.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle_libarchive_readonly.desktop
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/kerfuffle_libarchive.desktop.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle_libarchive.desktop
)

set(kerfuffle_libarchive_SRCS libarchivehandler.cpp)

kde4_add_plugin(kerfuffle_libarchive ${kerfuffle_libarchive_SRCS})

target_link_libraries(kerfuffle_libarchive ${KDE4_KDECORE_LIBS} ${KDE4_KARCHIVE_LIBS} kerfuffle)

install(TARGETS kerfuffle_libarchive  DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )


########### install files ###############

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle_libarchive.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle_libarchive_readonly.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)

set(SUPPORTED_ARK_MIMETYPES "${SUPPORTED_ARK_MIMETYPES}${SUPPORTED_LIBARCHIVE_READWRITE_MIMETYPES}${SUPPORTED_LIBARCHIVE_READONLY_MIMETYPES}" PARENT_SCOPE)
