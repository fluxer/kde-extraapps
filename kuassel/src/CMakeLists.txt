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

# Needed for showing the cli option if appropriate
if (HAVE_SYSLOG)
    add_definitions(-DHAVE_SYSLOG)
endif()

include_directories(${KDE4_INCLUDES})

add_executable(quassel WIN32 common/main.cpp qtui/monoapplication.cpp ${CLIENT_DEPS} ${CORE_DEPS} ${COMMON_DEPS})
qt4_use_modules(quassel Core Gui Network ${CLIENT_QT_MODULES} ${CORE_QT_MODULES})
add_dependencies(quassel po)
set_target_properties(quassel PROPERTIES
			      OUTPUT_NAME ../quassel)
target_link_libraries(quassel mod_qtui mod_uisupport mod_client mod_core mod_common ${COMMON_LIBRARIES} ${CLIENT_LIBRARIES} ${QUASSEL_SSL_LIBRARIES} ${QT_QTMAIN_LIBRARY})
install(TARGETS quassel RUNTIME DESTINATION ${BIN_INSTALL_DIR})
