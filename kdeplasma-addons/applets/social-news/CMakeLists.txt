project(plasma-opendesktop-activities)


set(activities_SRCS
    activities.cpp
    activitylist.cpp
    activitywidget.cpp
    contactimage.cpp
)

kde4_add_plugin(plasma_applet_opendesktop_activities ${activities_SRCS})

target_link_libraries(
    plasma_applet_opendesktop_activities
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS plasma_applet_opendesktop_activities DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-opendesktop-activities.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES plasma-applet-opendesktop-activities.notifyrc DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma-applet-opendesktop-activities/)
