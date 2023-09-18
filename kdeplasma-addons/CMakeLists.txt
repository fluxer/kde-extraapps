project(kdeplasma-addons)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(GenerateExportHeader)
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

kde4_optional_find_package(KDE4Workspace)
set_package_properties(KDE4Workspace PROPERTIES
    DESCRIPTION "KDE base workspace libraries"
    URL "https://osdn.net/projects/kde/"
    PURPOSE "Needed for building several Plasma plugins"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

add_subdirectory(libs)
add_subdirectory(applets)
add_subdirectory(dataengines)
add_subdirectory(runners)
add_subdirectory(wallpapers)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
