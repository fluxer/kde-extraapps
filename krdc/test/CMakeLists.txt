
if(NOT NDEBUG)
    set(BUILD_TEST true)
endif(NOT NDEBUG)
if(BUILD_TEST)

    include_directories(
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set(testplugin_SRCS
        testviewfactory.cpp
        testview.cpp
    )

    kde4_add_plugin(krdc_testplugin ${testplugin_SRCS})

    target_link_libraries(krdc_testplugin 
        ${KDE4_KDECORE_LIBS}
        ${KDE4_KDEUI_LIBS}
        krdccore
    )
    install(TARGETS krdc_testplugin DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

    install(FILES krdc_test.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
endif(BUILD_TEST)
