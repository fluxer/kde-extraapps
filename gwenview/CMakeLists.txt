project(gwenview)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(GenerateExportHeader)
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5003)

## Dependencies
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
set_package_properties(Threads PROPERTIES
    PURPOSE "Required for asynchorouns functions and methods execution"
    DESCRIPTION "Platform dependent threads implementation"
    URL ""
    TYPE REQUIRED
)

find_package(KDE4Workspace)
set_package_properties(KDE4Workspace PROPERTIES
    DESCRIPTION "KDE base workspace libraries"
    URL "https://osdn.net/projects/kde/"
    PURPOSE "Standard file management features"
    TYPE REQUIRED
)

## Global settings

include_directories(
    ${QDBUS_INCLUDE_DIRS}
    # for gwenview_export.h
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/lib
)

## dirs to build
add_subdirectory(lib)
add_subdirectory(app)
add_subdirectory(importer)
add_subdirectory(part)
if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
add_subdirectory(icons)
add_subdirectory(images)
add_subdirectory(cursors)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
