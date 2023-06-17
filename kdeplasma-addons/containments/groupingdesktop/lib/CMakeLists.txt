
set(groupingcontainment_SRCS
    abstractgroup.cpp
    groupinfo.cpp
    groupfactory.cpp
    groupingcontainment.cpp
    handle.cpp
    freehandle.cpp
    gridhandle.cpp
    spacer.cpp

    groups/floatinggroup.cpp
    groups/gridgroup.cpp
    groups/gridmanager.cpp
    groups/flowgroup.cpp
    groups/stackinggroup.cpp
    groups/tabbinggroup.cpp

    groupexplorer/explorerwindow.cpp
    groupexplorer/groupexplorer.cpp
    groupexplorer/groupiconlist.cpp
    groupexplorer/groupicon.cpp

    groups/tabbinggroup_config.ui
)
add_library(plasma_groupingcontainment SHARED ${groupingcontainment_SRCS})
target_link_libraries(plasma_groupingcontainment KDE4::plasma)

generate_export_header(plasma_groupingcontainment)

set_target_properties(plasma_groupingcontainment PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

install(
    TARGETS plasma_groupingcontainment
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)
