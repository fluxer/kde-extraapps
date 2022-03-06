project(skanlite)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(SKANLITE_VERSION_MAJOR "1")
set(SKANLITE_VERSION_MINOR "1")
set(SKANLITE_VERSION_STRING "${SKANLITE_VERSION_MAJOR}.${SKANLITE_VERSION_MINOR}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/src/version.h
)

add_subdirectory(src)
