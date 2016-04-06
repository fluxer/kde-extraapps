project(akregator)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.18.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
)

include(CheckTypeSize)
check_type_size("long" SIZEOF_LONG)
configure_file(config-akregator.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-akregator.h)

add_subdirectory(export)
add_subdirectory(interfaces)
add_subdirectory(plugins)
add_subdirectory(configuration)
add_subdirectory(src)
