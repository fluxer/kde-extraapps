project(ktimer)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set(ktimer_SRCS main.cpp ktimer.cpp prefwidget.ui)

add_executable(ktimer ${ktimer_SRCS})
target_link_libraries(ktimer  ${KDE4_KIO_LIBS})

install(
    TARGETS ktimer
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

########### install files ###############

install(
    PROGRAMS ktimer.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
