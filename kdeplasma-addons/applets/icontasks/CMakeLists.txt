set(tasks_SRCS
    windowtaskitem.cpp
    tasks.cpp
    taskitemlayout.cpp
    abstracttaskitem.cpp
    taskgroupitem.cpp
    applauncheritem.cpp
    jobmanager.cpp
    dockmanager.cpp
    dockitem.cpp
    dockhelper.cpp
    dockconfig.cpp
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
    dockconfig.ui
)

# Generate DBUS XML files, would like to use
# qt4_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/dockmanager.h net.launchpad.DockManager.xml)
# qt4_generate_dbus_interface(${CMAKE_CURRENT_SOURCE_DIR}/dockitem.h net.launchpad.DockItem.xml)
# ...but this does not work, as we also depend on the xml files in qt4_add_dbus_adaptor :-(
# So, need to manually create via:
#
# qdbuscpp2xml -M -P -S dockmanager.h -o net.launchpad.DockManager.xml
# qdbuscpp2xml -M -P -S dockitem.h -o net.launchpad.DockItem.xml

qt4_add_dbus_adaptor(tasks_SRCS net.launchpad.DockManager.xml dockmanager.h DockManager)
qt4_add_dbus_adaptor(tasks_SRCS net.launchpad.DockItem.xml dockitem.h DockItem)

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
install(FILES mediabuttonsrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/${CMAKE_PROJECT_NAME})
