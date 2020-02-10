project(print-manager)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

find_package(Cups "1.5" REQUIRED)
include(MacroLibrary)

# Print-Manager version
set(PM_VERSION "0.3.0" CACHE STRING "Print Manager version")

# Generate config.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/libkcups
    ${CUPS_INCLUDE_DIR}
)

add_subdirectory(libkcups)
add_subdirectory(configure-printer)
add_subdirectory(add-printer)
add_subdirectory(printer-manager-kcm)
add_subdirectory(printqueue)
add_subdirectory(print-manager-kded)
add_subdirectory(plasmoid)
add_subdirectory(declarative-plugins)
