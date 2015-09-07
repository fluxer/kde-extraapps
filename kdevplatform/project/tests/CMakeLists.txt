# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/project
)

set(projectmodeltest_SRCS
    projectmodeltest.cpp
)

kde4_add_test(kdevplatform-projectmodeltest ${projectmodeltest_SRCS})
target_link_libraries(kdevplatform-projectmodeltest
    ${QT_QTTEST_LIBRARY}
    kdevplatforminterfaces
    kdevplatformproject
    kdevplatformlanguage
    kdevplatformtests
)

set(projectmodelperformancetest_SRCS
    projectmodelperformancetest.cpp
)
kde4_add_manual_test(kdevplatform-projectmodelperformancetest ${projectmodelperformancetest_SRCS})
target_link_libraries(kdevplatform-projectmodelperformancetest
    ${QT_QTTEST_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    kdevplatforminterfaces
    kdevplatformproject
    kdevplatformlanguage
    kdevplatformtests
)
