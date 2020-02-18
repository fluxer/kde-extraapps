macro_optional_find_package(IBus 1.1.0)
set_package_properties(IBus PROPERTIES
    DESCRIPTION "Intelligent Input Bus for Linux/Unix"
    URL "https://github.com/ibus/ibus"
    TYPE OPTIONAL
    PURPOSE "Required by input method Plasma appelt"
)

macro_optional_find_package(SCIM)
set_package_properties(SCIM PROPERTIES
    DESCRIPTION "Smart Common Input Method platform"
    URL "https://github.com/scim-im/scim"
    TYPE OPTIONAL
    PURPOSE "Required by input method Plasma appelt"
)

include_directories(
    ${KDE4_INCLUDES} 
    ${PLASMA_INCLUDE_DIR} 
    ${X11_INCLUDE_DIR}
    ${X11_Xrender_INCLUDE_PATH}
)

add_subdirectory(src)
add_subdirectory(backend)
