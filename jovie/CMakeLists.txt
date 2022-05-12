project(kttsd)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.22.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

include_directories(${CMAKE_SOURCE_DIR})

find_package(Speechd)
set_package_properties(Speechd PROPERTIES
    URL "http://www.freebsoft.org/speechd"
    DESCRIPTION "Speech Dispatcher provides a high-level device independent layer for speech synthesis"
    PURPOSE "Jovie requires speech dispatcher."
    TYPE REQUIRED
)

find_program(XSLTPROC_EXECUTABLE NAMES xsltproc)
add_feature_info(xsltproc
    XSLTPROC_EXECUTABLE
    "SSML support in Jovie"
)

if(NOT Q_WS_X11)
    message(FATAL_ERROR "Jovie requires a X11-based system.")
endif()

configure_file(
    config-jovie.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-jovie.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/libkttsd
    ${CMAKE_CURRENT_SOURCE_DIR}/libkttsd
)

add_subdirectory(libkttsd)
add_subdirectory(filters)
add_subdirectory(jovie)
add_subdirectory(kcmkttsmgr)
add_subdirectory(icons)
