project(kcron)

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

add_subdirectory(src) 


