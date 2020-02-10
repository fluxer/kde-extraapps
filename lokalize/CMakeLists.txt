project(lokalize)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

find_package(HUNSPELL)
set_package_properties(HUNSPELL PROPERTIES
    DESCRIPTION "Library used for stemming"
    URL         "http://hunspell.sourceforge.net/"
    TYPE        REQUIRED 
    PURPOSE     "Required to build Lokalize."
)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
add_subdirectory(src)
add_subdirectory(scripts)
add_subdirectory(icons)
#add_subdirectory(strigi-analyzer)
