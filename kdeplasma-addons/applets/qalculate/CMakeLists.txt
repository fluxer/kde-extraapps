project(plasma-qalculate)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")
 
include_directories(${QALCULATE_INCLUDE_DIR})

# We add our source code here
set(qalculate_SRCS 
    qalculate_applet.cpp
    qalculate_labels.cpp
    qalculate_settings.cpp
    qalculate_engine.cpp
    qalculate_history.cpp
    outputlabel.cpp
    qalculate_graphicswidget.cpp    
)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_qalculate ${qalculate_SRCS})
target_link_libraries(plasma_applet_qalculate 
    KDE4::plasma
    KDE4::kdeui
    KDE4::solid
    KDE4::kio
    ${QALCULATE_LIBRARIES}
)
 
install(TARGETS plasma_applet_qalculate
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-qalculate.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

add_subdirectory(icons)
