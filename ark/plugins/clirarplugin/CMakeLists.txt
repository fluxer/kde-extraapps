########### next target ###############

set(SUPPORTED_CLIRAR_MIMETYPES "application/x-rar;")

set(kerfuffle_clirar_SRCS cliplugin.cpp)

kde4_add_plugin(kerfuffle_clirar ${kerfuffle_clirar_SRCS})

target_link_libraries(kerfuffle_clirar  ${KDE4_KDECORE_LIBS} kerfuffle )

configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/kerfuffle_clirar.desktop.cmake
            ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle_clirar.desktop
)

########### install files ###############

install(TARGETS kerfuffle_clirar  DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kerfuffle_clirar.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()

set(SUPPORTED_ARK_MIMETYPES "${SUPPORTED_ARK_MIMETYPES}${SUPPORTED_CLIRAR_MIMETYPES}" PARENT_SCOPE)
