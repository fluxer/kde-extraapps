
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9515 )

########### next target ###############

set(kdevexecute_PART_SRCS
    projecttargetscombobox.cpp
    executeplugin.cpp
    nativeappconfig.cpp
    nativeappjob.cpp
    nativeappconfig.ui
)

kde4_add_plugin(kdevexecute ${kdevexecute_PART_SRCS}  )
target_link_libraries(kdevexecute 
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KCMUTILS_LIBS}
    kdevplatforminterfaces 
    kdevplatformutil 
    kdevplatformproject 
    kdevplatformoutputview)

install(TARGETS kdevexecute DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############
install(FILES iexecuteplugin.h DESTINATION ${INCLUDE_INSTALL_DIR}/kdevplatform/execute COMPONENT Devel)
configure_file(kdevexecute.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevexecute.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevexecute.desktop DESTINATION ${SERVICES_INSTALL_DIR})
