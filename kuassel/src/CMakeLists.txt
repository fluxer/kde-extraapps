# Builds the three main targets

add_subdirectory(common)
include_directories(BEFORE common)
add_subdirectory(core)
include_directories(BEFORE core)
add_subdirectory(client)
add_subdirectory(uisupport)
add_subdirectory(qtui)
include_directories(BEFORE client)
include_directories(BEFORE uisupport)
include_directories(BEFORE qtui)

include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}) # for version.gen

add_executable(kuassel WIN32 common/main.cpp qtui/monoapplication.cpp
    $<TARGET_OBJECTS:mod_qtui>
    $<TARGET_OBJECTS:mod_uisupport>
    $<TARGET_OBJECTS:mod_client>
    $<TARGET_OBJECTS:mod_core>
    $<TARGET_OBJECTS:mod_common>
    ${CLIENT_DEPS} ${CORE_DEPS} ${COMMON_DEPS})
set_target_properties(kuassel PROPERTIES OUTPUT_NAME ../kuassel)

target_link_libraries(kuassel
    ${COMMON_LIBRARIES}
    ${CLIENT_LIBRARIES}
    ${CORE_LIBRARIES}
    ${QTUI_LIBRARIES}
    ${UISUPPORT_LIBRARIES}
    ${QUASSEL_SSL_LIBRARIES}
    ${QT_QTMAIN_LIBRARY}
)

install(TARGETS kuassel RUNTIME DESTINATION ${BIN_INSTALL_DIR})
