
include_directories(
    ${CMAKE_SOURCE_DIR}/miniboost
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDevPlatform_SOURCE_DIR}
)

add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9512 )

########### next target ###############

set(kdevclassbrowser_PART_SRCS
    classbrowserplugin.cpp
    classmodel.cpp
    classmodelnode.cpp
    classmodelnodescontroller.cpp
    classwidget.cpp
    classtree.cpp
    documentclassesfolder.cpp
    projectfolder.cpp
    allclassesfolder.cpp
)

kde4_add_plugin(kdevclassbrowser ${kdevclassbrowser_PART_SRCS})
target_link_libraries(kdevclassbrowser ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS} kdevplatformutil kdevplatformlanguage kdevplatforminterfaces kdevplatformproject)

install(TARGETS kdevclassbrowser DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

configure_file(kdevclassbrowser.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevclassbrowser.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevclassbrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kdevclassbrowser.rc DESTINATION ${DATA_INSTALL_DIR}/kdevclassbrowser)
