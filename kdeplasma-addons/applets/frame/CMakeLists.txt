project(plasma-frame)

set(frame_SRCS
    frame.cpp
    slideshow.cpp
    picture.cpp
    imagescaler.cpp
    imageloader.cpp
    configdialog.cpp
    imageSettings.ui
    appearanceSettings.ui
)

kde4_add_plugin(plasma_applet_frame ${frame_SRCS})

include_directories( ${KEXIV2_INCLUDE_DIR} )
set_source_files_properties(picture.cpp imageloader.cpp PROPERTIES
                            COMPILE_FLAGS -DHAVE_KEXIV2)
target_link_libraries(plasma_applet_frame
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KEXIV2_LIBS}
)

install(TARGETS plasma_applet_frame DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-frame-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#install(FILES picture-frame-default.jpg DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

install(FILES picture-frame-default.jpg DESTINATION ${DATA_INSTALL_DIR}/plasma-applet-frame/)
