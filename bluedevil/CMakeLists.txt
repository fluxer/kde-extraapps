project(bluedevil)

find_package(KDE4 4.14.3 REQUIRED)

set(CMAKE_BLUEDEVIL_VERSION_MAJOR 2)
set(CMAKE_BLUEDEVIL_VERSION_MINOR 1)
set(CMAKE_BLUEDEVIL_VERSION_PATCH 0)
set(CMAKE_BLUEDEVIL_VERSION_STRING  "${CMAKE_BLUEDEVIL_VERSION_MAJOR}.${CMAKE_BLUEDEVIL_VERSION_MINOR}.${CMAKE_BLUEDEVIL_VERSION_PATCH}")
configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

include(KDE4Defaults)
include(MacroOptionalAddSubdirectory)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
find_package(SharedMimeInfo REQUIRED)

include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} libbluedevil)

add_subdirectory(src)

add_subdirectory( libbluedevil )
