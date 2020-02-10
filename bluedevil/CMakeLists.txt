project(bluedevil)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(CMAKE_BLUEDEVIL_VERSION_MAJOR 2)
set(CMAKE_BLUEDEVIL_VERSION_MINOR 1)
set(CMAKE_BLUEDEVIL_VERSION_PATCH 1)
set(CMAKE_BLUEDEVIL_VERSION_STRING  "${CMAKE_BLUEDEVIL_VERSION_MAJOR}.${CMAKE_BLUEDEVIL_VERSION_MINOR}.${CMAKE_BLUEDEVIL_VERSION_PATCH}")

configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

find_package(SharedMimeInfo REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR} libbluedevil)

add_subdirectory(src)

add_subdirectory(libbluedevil)
