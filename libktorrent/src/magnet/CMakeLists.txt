if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set (magnet_HDR
    magnetdownloader.h
    magnetlink.h
    metadatadownload.h
)

install(FILES ${magnet_HDR} DESTINATION ${INCLUDE_INSTALL_DIR}/libktorrent/magnet COMPONENT Devel)
