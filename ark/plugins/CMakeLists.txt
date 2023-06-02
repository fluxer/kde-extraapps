if(LibArchive_FOUND)
    add_subdirectory( libarchive )
endif()

add_subdirectory( libsinglefileplugin )

set(SUPPORTED_ARK_MIMETYPES "${SUPPORTED_ARK_MIMETYPES}" PARENT_SCOPE)
