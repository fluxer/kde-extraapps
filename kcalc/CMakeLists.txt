project(kcalc)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${KDE4_ENABLE_EXCEPTIONS}")

add_definitions (-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

find_package(GMP)
macro_log_feature( GMP_FOUND "GMP" "The GNU Multiple Precision Arithmetic Library" "http://gmplib.org/" FALSE "" "Required for building KCalc.")

include(CheckTypeSize)
include(CheckIncludeFiles)

check_include_files(ieeefp.h     HAVE_IEEEFP_H)
check_type_size("signed long"    SIZEOF_SIGNED_LONG)
check_type_size("unsigned long"  SIZEOF_UNSIGNED_LONG)

configure_file(config-kcalc.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kcalc.h )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/knumber ${GMP_INCLUDE_DIR} )

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

kde4_add_kcfg_files(kcalc_SRCS kcalc_settings.kcfgc )

add_executable( kcalc ${kcalc_SRCS})

target_link_libraries(kcalc ${QT_QTXML_LIBRARY} ${KDE4_KDEUI_LIBS} ${GMP_LIBRARIES} ${MPFR_LIBRARIES})

install(TARGETS kcalc  ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS kcalc.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kcalc.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kcalcui.rc  DESTINATION ${DATA_INSTALL_DIR}/kcalc)
install( FILES scienceconstants.xml  DESTINATION ${DATA_INSTALL_DIR}/kcalc)

kde4_install_icons( ${ICON_INSTALL_DIR}   )
