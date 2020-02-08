if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set (datachecker_HDR
    datacheckerthread.h
    datachecker.h
    singledatachecker.h
    multidatachecker.h
    datacheckerjob.h
)

install(FILES ${datachecker_HDR}
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/libktorrent/datachecker
    COMPONENT Devel
)