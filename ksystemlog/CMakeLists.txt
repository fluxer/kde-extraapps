project(KSystemLog)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)


if(ENABLE_TESTING)
    set(CMAKE_C_VISIBILITY_PRESET "default")
    set(CMAKE_CXX_VISIBILITY_PRESET "default")
endif()

########### Subfolders ##########

add_subdirectory(src)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
