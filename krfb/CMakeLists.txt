project(krfb)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

find_package(LibVNCServer REQUIRED)

kde4_bool_to_01(X11_Xdamage_FOUND HAVE_XDAMAGE)
kde4_bool_to_01(X11_XShm_FOUND HAVE_XSHM)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/krfb
    ${CMAKE_CURRENT_SOURCE_DIR}/krfb
    ${CMAKE_CURRENT_SOURCE_DIR}/krfb/ui
)

if(Q_WS_X11)
    if(NOT X11_XTest_FOUND)
        message(FATAL_ERROR "krfb requires the libXTest (http://xorg.freedesktop.org) to be built")
    endif(NOT X11_XTest_FOUND)
endif(Q_WS_X11)

add_subdirectory(krfb)
add_subdirectory(framebuffers)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
