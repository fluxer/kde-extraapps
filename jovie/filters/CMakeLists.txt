add_subdirectory(stringreplacer) 
add_subdirectory(xmltransformer) 
add_subdirectory(talkerchooser) 

########### next target ###############

if(ENABLE_TESTING)
    set(testfilter_SRCS main.cpp )

    kde4_add_manual_test(testfilter main.cpp)

    target_link_libraries(testfilter  ${KDE4_KDEUI_LIBS} kttsd)
endif()

########### install files ###############

install(
    FILES jovie_filterplugin.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)
