if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set (diskio_HDR  
    cache.h
    cachefile.h
    singlefilecache.h
    preallocationthread.h
    preallocationjob.h
    movedatafilesjob.h
    deletedatafilesjob.h
    chunkmanager.h
    dndfile.h
    chunk.h
    multifilecache.h
    piecedata.h
)

install(FILES ${diskio_HDR} DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/libktorrent/diskio COMPONENT Devel)