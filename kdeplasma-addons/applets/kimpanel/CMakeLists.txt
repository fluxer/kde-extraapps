include_directories(
    ${KDE4_INCLUDES} 
    ${PLASMA_INCLUDE_DIR} 
    ${X11_INCLUDE_DIR}
    ${X11_Xrender_INCLUDE_PATH}
)

add_subdirectory(src)
add_subdirectory(backend)
