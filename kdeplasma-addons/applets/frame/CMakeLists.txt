project(plasma-frame)

set(frame_SRCS
    frame.cpp
    slideshow.cpp
    picture.cpp
    configdialog.cpp
    imageSettings.ui
    appearanceSettings.ui
)

kde4_add_plugin(plasma_applet_frame ${frame_SRCS})

include_directories(${KEXIV2_INCLUDE_DIR})
target_link_libraries(plasma_applet_frame
    KDE4::plasma
    KDE4::kio
    KDE4::kfile
    KDE4::kexiv2
)

install(TARGETS plasma_applet_frame DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-frame-default.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
#install(FILES picture-frame-default.jpg DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

install(FILES picture-frame-default.jpg DESTINATION ${KDE4_DATA_INSTALL_DIR}/plasma-applet-frame/)
