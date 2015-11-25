project(kdegraphics-thumbnailers)

add_definitions(
    ${QT_DEFINITIONS}
    ${KDE4_DEFINITIONS}
    -DQT_USE_FAST_CONCATENATIO
    -DQT_USE_FAST_OPERATOR_PLUS
)
include_directories(${KDE4_INCLUDES})

macro_optional_find_package(GettextPO)

set_package_properties(GettextPO PROPERTIES
    DESCRIPTION "A library for processing PO files"
    URL "www.gnu.org/software/gettext"
    PURPOSE "Required to build the PO thumbnailer"
)

macro_optional_add_subdirectory(audio)
macro_optional_add_subdirectory(ps)
macro_optional_add_subdirectory(raw)

if(GETTEXTPO_FOUND)
    macro_optional_add_subdirectory(po)
endif(GETTEXTPO_FOUND)

