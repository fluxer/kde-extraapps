project(bluedevil)

set(CMAKE_BLUEDEVIL_VERSION_MAJOR 2)
set(CMAKE_BLUEDEVIL_VERSION_MINOR 1)
set(CMAKE_BLUEDEVIL_VERSION_PATCH 1)
set(CMAKE_BLUEDEVIL_VERSION_STRING  "${CMAKE_BLUEDEVIL_VERSION_MAJOR}.${CMAKE_BLUEDEVIL_VERSION_MINOR}.${CMAKE_BLUEDEVIL_VERSION_PATCH}")
configure_file(version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

include(MacroOptionalAddSubdirectory)

find_package(SharedMimeInfo REQUIRED)

include_directories(${KDE4_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} libbluedevil)

add_subdirectory(src)

add_subdirectory( libbluedevil )
