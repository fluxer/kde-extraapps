set(tasks_SRCS
    windowtaskitem.cpp
    tasks.cpp
    taskitemlayout.cpp
    abstracttaskitem.cpp
    taskgroupitem.cpp
    applauncheritem.cpp
    jobmanager.cpp
    mediabuttons.cpp
    unity.cpp
    unityitem.cpp 
    dbusstatus.cpp
    recentdocuments.cpp
    tooltips/tooltipcontent.cpp
    tooltips/tooltip.cpp
    tooltips/tooltipmanager.cpp
    tooltips/windowpreview.cpp
    tooltips/dialogshadows.cpp
    tooltips/dialogshadows_p.h
    appearanceconfig.ui
    behaviourconfig.ui
)

qt4_add_dbus_interface(tasks_SRCS org.mpris.MediaPlayer2.Player.xml playerv2interface)

set_source_files_properties(org.freedesktop.MediaPlayer.player.xml PROPERTIES INCLUDE "dbusstatus.h")
qt4_add_dbus_interface(tasks_SRCS org.freedesktop.MediaPlayer.player.xml playerv1interface)

kde4_add_plugin(plasma_applet_icontasks ${tasks_SRCS})

target_link_libraries(plasma_applet_icontasks
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${DBUSMENUQT_LIBRARIES}
    ${KDE4WORKSPACE_TASKMANAGER_LIBS}
)

if(Q_WS_X11)
    target_link_libraries(plasma_applet_icontasks ${X11_X11_LIB})
endif(Q_WS_X11)

include_directories(${CMAKE_BINARY_DIR} ${DBUSMENUQT_INCLUDE_DIR})

install(TARGETS plasma_applet_icontasks DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-icontasks.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES badge.svgz launcherseparator.svgz progress.svgz indicators.svgz dropindicators.svgz DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/icontasks/)
install(FILES mediabuttonsrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kdeplasma-addons)
