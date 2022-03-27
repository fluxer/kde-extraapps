add_subdirectory(kio)
if (LIBMMS_FOUND)
    add_subdirectory(mmsthreads)
endif (LIBMMS_FOUND)
if (LIBTORRENT_FOUND)
    add_subdirectory(torrent)
endif (LIBTORRENT_FOUND)

add_subdirectory(checksumsearch)

SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}" PARENT_SCOPE)
