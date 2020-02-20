project(kmix)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(Alsa)
set_package_properties(Alsa PROPERTIES
    DESCRIPTION "Advanced Linux Sound Architecture"
    URL "https://alsa-project.org/wiki/Main_Page"
    PURPOSE "Needed for KMix sound feedback"
)

macro_optional_find_package(PulseAudio 0.9.16)
set_package_properties(PulseAudio PROPERTIES
    DESCRIPTION "PulseAudio Audio Server"
    URL "http://www.pulseaudio.org/"
    PURPOSE "Needed to let KMix control PulseAudio"
)

macro_optional_find_package(Canberra)
set_package_properties(Canberra PROPERTIES
    DESCRIPTION "libcanberra audio library"
    URL "http://0pointer.de/lennart/projects/libcanberra/"
    PURPOSE "Needed for KMix sound feedback"
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=67100)

include_directories(/usr/lib/oss/include)

add_subdirectory( pics ) 
add_subdirectory( profiles ) 
#add_subdirectory( tests )

if (ALSA_FOUND)
    add_definitions(-DHAVE_ALSA)
    include_directories(${ALSA_INCLUDES})
endif (ALSA_FOUND)

if (PULSEAUDIO_FOUND)
    add_definitions(-DHAVE_PULSE ${PULSEAUDIO_DEFINITIONS})
    include_directories(${PULSEAUDIO_INCLUDE_DIR})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
    add_definitions(-DHAVE_CANBERRA)
    include_directories(${CANBERRA_INCLUDE_DIRS})
endif (CANBERRA_FOUND)


set(kmix_adaptor_SRCS
	dbus/dbusmixerwrapper.cpp
	dbus/dbusmixsetwrapper.cpp
	dbus/dbuscontrolwrapper.cpp)
qt4_add_dbus_adaptor( kmix_adaptor_SRCS dbus/org.kde.kmix.control.xml
	dbus/dbuscontrolwrapper.h DBusControlWrapper )
qt4_add_dbus_adaptor( kmix_adaptor_SRCS dbus/org.kde.kmix.mixer.xml
	dbus/dbusmixerwrapper.h DBusMixerWrapper )
qt4_add_dbus_adaptor( kmix_adaptor_SRCS dbus/org.kde.kmix.mixset.xml
	dbus/dbusmixsetwrapper.h DBusMixSetWrapper )

set(kmix_backend_SRCS
    backends/mixer_backend.cpp
    backends/mixer_mpris2.cpp
)

if (ALSA_FOUND)
    set(kmix_backend_SRCS ${kmix_backend_SRCS}
        backends/mixer_alsa9.cpp )
endif (ALSA_FOUND)

if (PULSEAUDIO_FOUND)
    set(kmix_backend_SRCS ${kmix_backend_SRCS}
        backends/mixer_pulse.cpp )
endif (PULSEAUDIO_FOUND)

set(kmix_SRCS ${kmix_adaptor_SRCS} ${kmix_backend_SRCS}
    apps/main.cpp 
    apps/kmix.cpp 
    apps/KMixApp.cpp
    gui/kmixdockwidget.cpp 
    gui/kmixprefdlg.cpp 
    gui/viewbase.cpp 
    gui/viewdockareapopup.cpp 
    gui/viewsliders.cpp 
    gui/mixdevicewidget.cpp 
    gui/mdwmoveaction.cpp
    gui/mdwslider.cpp 
    gui/mdwenum.cpp 
    gui/kmixerwidget.cpp 
    gui/ksmallslider.cpp
    gui/verticaltext.cpp
    gui/volumeslider.cpp 
    gui/kmixtoolbox.cpp 
    gui/dialogaddview.cpp 
    gui/dialogviewconfiguration.cpp 
    gui/dialogselectmaster.cpp 
    gui/dialogchoosebackends.cpp 
    gui/guiprofile.cpp
    gui/osdwidget.cpp
    core/MediaController.cpp
    core/mixertoolbox.cpp
    core/kmixdevicemanager.cpp
    core/ControlManager.cpp
    #   core/ControlPool.cpp
    core/GlobalConfig.cpp
    core/MasterControl.cpp
    core/mixer.cpp
    core/mixset.cpp
    core/mixdevice.cpp
    core/mixdevicecomposite.cpp
    core/volume.cpp
)

add_executable(kmix ${kmix_SRCS})

target_link_libraries(kmix
    ${KDE4_SOLID_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${QT_QTXML_LIBRARY}
)

if (ALSA_FOUND)
    target_link_libraries(kmix ${ALSA_LIBRARIES})
endif (ALSA_FOUND)

if (PULSEAUDIO_FOUND)
    target_link_libraries(kmix ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
    target_link_libraries(kmix ${CANBERRA_LIBRARIES})
endif (CANBERRA_FOUND)

install(
    TARGETS kmix
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

########### next target ###############

set(kded_kmixd_SRCS ${kmix_adaptor_SRCS} ${kmix_backend_SRCS}
    apps/kmixd.cpp 
    core/ControlManager.cpp
#    core/ControlPool.cpp
    core/GlobalConfig.cpp
    core/MasterControl.cpp
    core/MediaController.cpp
    core/mixer.cpp 
    core/mixset.cpp 
    core/mixdevice.cpp 
    core/volume.cpp
    core/mixertoolbox.cpp 
    core/kmixdevicemanager.cpp
)

#qt4_add_dbus_adaptor(kded_kmixd_SRCS org.kde.KMixD.xml kmixd.h Mixer) 
 
kde4_add_plugin(kded_kmixd ${kded_kmixd_SRCS})


target_link_libraries(kded_kmixd
    ${KDE4_KDEUI_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QT_QTXML_LIBRARY}
)

if (ALSA_FOUND)
    target_link_libraries(kded_kmixd ${ALSA_LIBRARIES})
endif (ALSA_FOUND)

if (PULSEAUDIO_FOUND)
    target_link_libraries(kded_kmixd ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
    target_link_libraries(kded_kmixd ${CANBERRA_LIBRARIES})
endif (CANBERRA_FOUND)

install(TARGETS kded_kmixd DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

#target_link_libraries( kmixd kded_kmixd )
#install(TARGETS kmixd DESTINATION ${KDE4_PLUGIN_INSTALL_DIR} )

install(FILES kmixd.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/kded)

########### next target ###############

set(kmixctrl_SRCS ${kmix_adaptor_SRCS} ${kmix_backend_SRCS}
    apps/kmixctrl.cpp 
    core/ControlManager.cpp
    #   core/ControlPool.cpp
    core/GlobalConfig.cpp
    core/MasterControl.cpp
    core/MediaController.cpp
    core/mixer.cpp 
    core/mixset.cpp 
    core/mixdevice.cpp 
    core/volume.cpp 
    core/mixertoolbox.cpp 
    core/kmixdevicemanager.cpp
)

# gui/guiprofile.cpp


add_executable(kmixctrl ${kmixctrl_SRCS})

target_link_libraries(kmixctrl
    ${KDE4_KDEUI_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QT_QTXML_LIBRARY}
)

if (ALSA_FOUND)
    target_link_libraries(kmixctrl ${ALSA_LIBRARIES})
endif (ALSA_FOUND)

if (PULSEAUDIO_FOUND)
    target_link_libraries(kmixctrl ${PULSEAUDIO_LIBRARY} ${PULSEAUDIO_MAINLOOP_LIBRARY})
endif (PULSEAUDIO_FOUND)

if (CANBERRA_FOUND)
    target_link_libraries(kmixctrl ${CANBERRA_LIBRARIES})
endif (CANBERRA_FOUND)

########### next target ###############
add_subdirectory( plasma )

install(
    TARGETS kmixctrl
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
    ${INSTALL_TARGETS_DEFAULT_ARGS}
)

install(PROGRAMS kmix.desktop DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR})
install(PROGRAMS apps/kmixremote DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES restore_kmix_volumes.desktop DESTINATION ${KDE4_AUTOSTART_INSTALL_DIR})
install(FILES kmix_autostart.desktop  DESTINATION ${KDE4_AUTOSTART_INSTALL_DIR})
install(FILES kmixui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kmix)
install(FILES kmixctrl_restore.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES dbus/org.kde.kmix.control.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})
install(FILES dbus/org.kde.kmix.mixer.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})
install(FILES dbus/org.kde.kmix.mixset.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})
