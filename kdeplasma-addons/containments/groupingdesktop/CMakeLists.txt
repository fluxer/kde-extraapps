project(groupingdesktop)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/lib
)

add_subdirectory(lib)
add_subdirectory(desktop)
add_subdirectory(panel)
add_subdirectory(griddesktop)
