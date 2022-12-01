project(kcalc)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDE4 4.23.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

find_package(GMP)
set_package_properties(GMP PROPERTIES
    DESCRIPTION "The GNU Multiple Precision Arithmetic Library"
    URL "http://gmplib.org/"
    PURPOSE "Required for building KCalc"
    TYPE REQUIRED
)

include(CheckTypeSize)

check_type_size("signed long"    SIZEOF_SIGNED_LONG)
check_type_size("unsigned long"  SIZEOF_UNSIGNED_LONG)

configure_file(
    config-kcalc.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-kcalc.h
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/knumber
    ${GMP_INCLUDE_DIR}
)

########### next target ###############

# Needs absolute paths due to the test program for knumber
set(libknumber_la_SRCS  
    ${kcalc_SOURCE_DIR}/knumber/knumber.cpp
    ${kcalc_SOURCE_DIR}/knumber/knumber_error.cpp
    ${kcalc_SOURCE_DIR}/knumber/knumber_float.cpp
    ${kcalc_SOURCE_DIR}/knumber/knumber_fraction.cpp
    ${kcalc_SOURCE_DIR}/knumber/knumber_integer.cpp
    ${kcalc_SOURCE_DIR}/knumber/knumber_operators.cpp
)

add_subdirectory( knumber )
# add_subdirectory( tests )

set(kcalc_SRCS ${libknumber_la_SRCS}
    kcalc.cpp
    bitbutton.cpp
    kcalc_bitset.cpp
    kcalc_button.cpp
    kcalc_const_button.cpp
    kcalc_const_menu.cpp
    kcalc_core.cpp
    kcalcdisplay.cpp
    stats.cpp
    kcalc.ui
    constants.ui
    colors.ui
    fonts.ui
    general.ui
)

kde4_add_kcfg_files(kcalc_SRCS kcalc_settings.kcfgc)

add_executable(kcalc ${kcalc_SRCS})

target_link_libraries(kcalc
    ${KDE4_KDEUI_LIBS}
    ${QT_QTXML_LIBRARY}
    ${GMP_LIBRARIES}
)

install(TARGETS kcalc ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(
    PROGRAMS kcalc.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)

install(
    FILES kcalc.kcfg
    DESTINATION ${KDE4_KCFG_INSTALL_DIR}
)

install(
    FILES kcalcui.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kcalc
)

install(
    FILES scienceconstants.xml
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kcalc
)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
