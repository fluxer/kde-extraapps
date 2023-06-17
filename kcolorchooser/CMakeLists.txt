project(kcolorchooser)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
)

set(kcolorchooser_SRCS kcolorchooser.cpp)

add_executable(kcolorchooser ${kcolorchooser_SRCS})

target_link_libraries(kcolorchooser KDE4::kdeui)

install(
    TARGETS kcolorchooser
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)
install(
    PROGRAMS kcolorchooser.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
