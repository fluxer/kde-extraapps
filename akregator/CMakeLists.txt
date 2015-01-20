project(akregator)

#search packages used by KDE
set(QT_MIN_VERSION "4.8.2")
find_package(KDE4 4.14.3 REQUIRED)
include (KDE4Defaults)

# KDE/QT definitions we want
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories (${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES})

include(CheckTypeSize)
check_type_size("long" SIZEOF_LONG)
configure_file(config-akregator.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-akregator.h)

add_subdirectory(export)
add_subdirectory(interfaces)
add_subdirectory(plugins)
add_subdirectory(configuration)
add_subdirectory(src)
