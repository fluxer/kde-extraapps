if(LibArchive_FOUND)
    add_subdirectory( libarchive )
else()
    add_subdirectory( karchiveplugin )
endif()

add_subdirectory( clirarplugin )
add_subdirectory( libsinglefileplugin )
add_subdirectory( clilhaplugin )

set(SUPPORTED_ARK_MIMETYPES "${SUPPORTED_ARK_MIMETYPES}" PARENT_SCOPE)
