
INCLUDE_DIRECTORIES(${SANE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/options
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
)

SET(ksane_LIB_SRCS
    widgets/gamma_disp.cpp
    widgets/labeled_gamma.cpp
    widgets/labeled_checkbox.cpp
    widgets/labeled_entry.cpp
    widgets/labeled_combo.cpp
    widgets/labeled_fslider.cpp
    widgets/labeled_slider.cpp
    widgets/ksane_button.cpp
    widgets/ksane_option_widget.cpp
    ksane_viewer.cpp
    selectionitem.cpp
    ksane_device_dialog.cpp
    ksane_find_devices_thread.cpp
    ksane_widget.cpp
    ksane_scan_thread.cpp
    ksane_preview_thread.cpp
    ksane_widget_private.cpp
    splittercollapser.cpp
    ksane_auth.cpp
    options/ksane_option.cpp
    options/ksane_opt_button.cpp
    options/ksane_opt_checkbox.cpp
    options/ksane_opt_entry.cpp
    options/ksane_opt_gamma.cpp
    options/ksane_opt_slider.cpp
    options/ksane_opt_fslider.cpp
    options/ksane_opt_combo.cpp
    )

add_library(ksane SHARED ${ksane_LIB_SRCS})

TARGET_LINK_LIBRARIES(ksane ${KDE4_KDEUI_LIBS}
                            ${SANE_LIBRARY}
                     )

SET_TARGET_PROPERTIES(ksane PROPERTIES
    VERSION ${KSANE_LIB_SO_VERSION_STRING}
    SOVERSION ${KSANE_LIB_SO_CUR_VERSION}
)

OPTION(COMPILE_VIEWER_TESTAPP "Compile a small test application for the viewer")

IF (COMPILE_VIEWER_TESTAPP)
    message("Compile testapp")
    add_executable(viewer_test ksane_viewer.cpp ksane_viewer_test.cpp selectionitem.cpp)
    target_link_libraries(viewer_test ${KDE4_KDEUI_LIBS})
ENDIF()

INSTALL(TARGETS ksane ${INSTALL_TARGETS_DEFAULT_ARGS} )

CONFIGURE_FILE(
    ../cmake/templates/KSaneConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/KSaneConfig.cmake
    @ONLY
)

INSTALL(
    FILES
    ksane.h
    libksane_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/libksane
    COMPONENT Devel
)

CONFIGURE_FILE(
    ${PROJECT_SOURCE_DIR}/cmake/templates/libksane.pc.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libksane.pc
)
INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libksane.pc
    DESTINATION ${KDE4_LIB_INSTALL_DIR}/pkgconfig
)

# Install find cmake script to the system for client applications.

INSTALL(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KSaneConfig.cmake
    DESTINATION ${KDE4_LIB_INSTALL_DIR}/cmake/KSane
)

KDE4_INSTALL_ICONS(${KDE4_ICON_INSTALL_DIR})

