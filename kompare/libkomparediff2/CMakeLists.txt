cmake_minimum_required(VERSION 2.6)
project(LibKompareDiff2)

include_directories (${PROJECT_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(komparediff2_SRCS
    kompareprocess.cpp 
    komparemodellist.cpp 
    diffmodellist.cpp 
    diffmodel.cpp 
    difference.cpp 
    diffhunk.cpp 
    diffsettings.cpp
    settingsbase.cpp
    parser.cpp 
    parserbase.cpp 
    cvsdiffparser.cpp 
    diffparser.cpp 
    perforceparser.cpp 
    stringlistpair.cpp
)


add_library(komparediff2 SHARED ${komparediff2_SRCS})

target_link_libraries(komparediff2
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)

set_target_properties(komparediff2 PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(komparediff2)

install(TARGETS komparediff2 EXPORT LibKompareDiff2Targets ${INSTALL_TARGETS_DEFAULT_ARGS})

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

install(
    FILES
    settingsbase.h
    diffsettings.h
    komparemodellist.h
    difference.h
    diffmodel.h
    diffmodellist.h
    marker.h
    kompare.h
    diffhunk.h
    ${CMAKE_CURRENT_BINARY_DIR}/komparediff2_export.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/libkomparediff2  COMPONENT Devel
)

# Config.cmake file.
configure_file( "${PROJECT_SOURCE_DIR}/LibKompareDiff2Config.cmake.in" "${CMAKE_BINARY_DIR}/LibKompareDiff2Config.cmake" @ONLY )

set(_LibKompareDiff2_CONFIG_DEST "${KDE4_LIB_INSTALL_DIR}/cmake/libkomparediff2")

install( FILES
        "${CMAKE_BINARY_DIR}/LibKompareDiff2Config.cmake"
        DESTINATION "${_LibKompareDiff2_CONFIG_DEST}" )
install( EXPORT LibKompareDiff2Targets
         DESTINATION "${_LibKompareDiff2_CONFIG_DEST}"
         NAMESPACE LibKompareDiff2Import__
         FILE LibKompareDiff2Targets.cmake )
