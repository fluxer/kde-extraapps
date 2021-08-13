project(kcm-about-distro)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.21.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(MAJOR_VERSION "1")
set(MINOR_VERSION "1")
set(PATCH_VERSION "0")
set(SUFFIX_VERSION "")

set(VERSION_STRING "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
set(VERSION_STRING_FULL "${VERSION_STRING}")

add_definitions(-std=c++0x)

add_subdirectory(src)

