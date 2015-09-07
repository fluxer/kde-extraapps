if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

configure_file( gdbinit ${CMAKE_CURRENT_BINARY_DIR}/gdbinit )

install( FILES helper.py qt4.py kde4.py
    ${CMAKE_CURRENT_BINARY_DIR}/gdbinit
    DESTINATION ${DATA_INSTALL_DIR}/kdevgdb/printers )
