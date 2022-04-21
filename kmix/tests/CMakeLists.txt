set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/.. )

set(kmix_adaptor_SRCS)
qt4_add_dbus_adaptor( kmix_adaptor_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/../dbus/org.kde.KMix.Mixer.xml
	${CMAKE_CURRENT_SOURCE_DIR}/../dbus/dbusmixerwrapper.cpp DBusMixerWrapper)


########### next target ###############
set(profiletest_SRCS profiletest.cpp )


add_executable(profiletest ${profiletest_SRCS} 
    ${CMAKE_CURRENT_SOURCE_DIR}/../gui/guiprofile.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/MasterControl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/mixer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/mixdevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/volume.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../backends/mixer_backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/mixset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/kmixdevicemanager.cpp 
    ${kmix_adaptor_SRCS}
)

target_link_libraries(profiletest ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTXML_LIBRARY})

if (ALSA_FOUND)
    target_link_libraries(profiletest ${ALSA_LIBRARIES})
endif (ALSA_FOUND)

########### next target ###############
set(dialogtest_SRCS dialogtest.cpp)

add_executable(dialogtest ${dialogtest_SRCS})
    
target_link_libraries(dialogtest ${KDE4_KDEUI_LIBS})
