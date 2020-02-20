set(kerfuffle_singlefile_SRCS singlefileplugin.cpp)
set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "")

# This MIME type was originally set in ark.desktop but is does not
# seem to be supported anywhere.  Assuming that, if it were supported,
# it would be here.
set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-compress;")

#
# GZip files
#
macro_optional_find_package(ZLIB)
set_package_properties(ZLIB PROPERTIES
    DESCRIPTION "General purpose data compression library"
    URL "http://www.zlib.net"
    PURPOSE "Required for the .gz format support in Ark"
)

if (ZLIB_FOUND)
    set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-gzip;")

    set(kerfuffle_libgz_SRCS gzplugin.cpp ${kerfuffle_singlefile_SRCS})

    kde4_add_plugin(kerfuffle_libgz ${kerfuffle_libgz_SRCS})

    target_link_libraries(kerfuffle_libgz
        ${KDE4_KDECORE_LIBS}
        kerfuffle
    )

    install(
        TARGETS kerfuffle_libgz
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kerfuffle_libgz.desktop
        DESTINATION
        ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

#
# Bzip2 files
#
macro_optional_find_package(BZip2)
set_package_properties(BZip2 PROPERTIES
    DESCRIPTION "A high-quality data compressor"
    URL "http://www.bzip.org"
    PURPOSE "Required for the .bz2 format support in Ark"
)

if (BZIP2_FOUND)
    set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-bzip;application/x-bzip2;")

    set(kerfuffle_libbz2_SRCS bz2plugin.cpp ${kerfuffle_singlefile_SRCS})

    kde4_add_plugin(kerfuffle_libbz2 ${kerfuffle_libbz2_SRCS})

    target_link_libraries(kerfuffle_libbz2
        ${KDE4_KDECORE_LIBS}
        kerfuffle
    )

    install(
        TARGETS kerfuffle_libbz2
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kerfuffle_libbz2.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

#
# LZMA files
#
macro_optional_find_package(LibLZMA)
set_package_properties(LibLZMA PROPERTIES
    DESCRIPTION "Legacy data compression software with high compression ratio"
    URL "http://tukaani.org/xz/"
    PURPOSE "Required for the .xz and .lzma format support in Ark"
)

if (LIBLZMA_FOUND)
    set(SUPPORTED_LIBSINGLEFILE_MIMETYPES "${SUPPORTED_LIBSINGLEFILE_MIMETYPES}application/x-lzma;application/x-xz;")

    set(kerfuffle_libxz_SRCS xzplugin.cpp ${kerfuffle_singlefile_SRCS})

    kde4_add_plugin(kerfuffle_libxz ${kerfuffle_libxz_SRCS})

    target_link_libraries(kerfuffle_libxz
        ${KDE4_KDECORE_LIBS}
        kerfuffle
    )

    install(
        TARGETS kerfuffle_libxz
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
    )

    install(
        FILES kerfuffle_libxz.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
    )
endif()

set(SUPPORTED_ARK_MIMETYPES "${SUPPORTED_ARK_MIMETYPES}${SUPPORTED_LIBSINGLEFILE_MIMETYPES}" PARENT_SCOPE)
