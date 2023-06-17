project(kmix)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

kde4_optional_find_package(ALSA)
set_package_properties(ALSA PROPERTIES
    DESCRIPTION "Advanced Linux Sound Architecture"
    URL "https://alsa-project.org/wiki/Main_Page"
    PURPOSE "Needed for KMix sound feedback"
)

configure_file(
    apps/kmixremote.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/kmixremote
    @ONLY
)

# NetBSD
find_library(LIBOSSAUDIO_LIBRARY NAMES ossaudio)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=67100)

include_directories(/usr/lib/oss/include)

add_subdirectory( pics ) 
add_subdirectory( profiles ) 
#add_subdirectory( tests )

if (ALSA_FOUND)
    add_definitions(-DHAVE_ALSA)
    include_directories(${ALSA_INCLUDE_DIR})
endif (ALSA_FOUND)

set(kmix_adaptor_SRCS
    dbus/dbusmixerwrapper.cpp
    dbus/dbusmixsetwrapper.cpp
    dbus/dbuscontrolwrapper.cpp
)
qt4_add_dbus_adaptor( kmix_adaptor_SRCS
    dbus/org.kde.kmix.control.xml
    dbus/dbuscontrolwrapper.h DBusControlWrapper
)
qt4_add_dbus_adaptor( kmix_adaptor_SRCS
    dbus/org.kde.kmix.mixer.xml
    dbus/dbusmixerwrapper.h DBusMixerWrapper
)
qt4_add_dbus_adaptor( kmix_adaptor_SRCS
    dbus/org.kde.kmix.mixset.xml
    dbus/dbusmixsetwrapper.h DBusMixSetWrapper
)

set(kmix_backend_SRCS
    backends/mixer_backend.cpp
    backends/mixer_mpris2.cpp
)

if(ALSA_FOUND)
    set(kmix_backend_SRCS ${kmix_backend_SRCS}
        backends/mixer_alsa9.cpp )
endif()

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
    KDE4::solid
    KDE4::kdeui
    KDE4::plasma
    ${QT_QTXML_LIBRARY}
    ${X11_X11_LIB}
)

if(ALSA_FOUND)
    target_link_libraries(kmix ${ALSA_LIBRARIES})
endif()

if(LIBOSSAUDIO_LIBRARY)
    target_link_libraries(kmix ${LIBOSSAUDIO_LIBRARY})
endif()

install(
    TARGETS kmix
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
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
    KDE4::kdeui
    KDE4::solid
    ${QT_QTXML_LIBRARY}
)

if(ALSA_FOUND)
    target_link_libraries(kded_kmixd ${ALSA_LIBRARIES})
endif()

if(LIBOSSAUDIO_LIBRARY)
    target_link_libraries(kded_kmixd ${LIBOSSAUDIO_LIBRARY})
endif()

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
    KDE4::kdeui
    KDE4::solid
    ${QT_QTXML_LIBRARY}
)

if(ALSA_FOUND)
    target_link_libraries(kmixctrl ${ALSA_LIBRARIES})
endif()

if(LIBOSSAUDIO_LIBRARY)
    target_link_libraries(kmixctrl ${LIBOSSAUDIO_LIBRARY})
endif()

########### next target ###############
add_subdirectory( plasma )

install(
    TARGETS kmixctrl
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

install(PROGRAMS kmix.desktop DESTINATION  ${KDE4_XDG_APPS_INSTALL_DIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/kmixremote DESTINATION ${KDE4_BIN_INSTALL_DIR})
install(FILES restore_kmix_volumes.desktop DESTINATION ${KDE4_AUTOSTART_INSTALL_DIR})
install(FILES kmix_autostart.desktop  DESTINATION ${KDE4_AUTOSTART_INSTALL_DIR})
install(FILES kmixui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kmix)
install(FILES kmixctrl_restore.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
install(FILES dbus/org.kde.kmix.control.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})
install(FILES dbus/org.kde.kmix.mixer.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})
install(FILES dbus/org.kde.kmix.mixset.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
