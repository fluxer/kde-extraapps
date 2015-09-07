set(apptemplate_DIRS
    cpp/CMake/cmake_qt4core
    cpp/CMake/cmake_qt4gui
    cpp/CMake/cmake_qt4core_qtest
    cpp/CMake/cmake_plaincpp
)

include(${CMAKE_SOURCE_DIR}/kdevplatform/cmake/modules/KDevPlatformMacros.cmake)
kdevplatform_add_app_templates(${apptemplate_DIRS})
