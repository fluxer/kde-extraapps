# Project Needs a name ofcourse
project(previewer)
 
# We add our source code here
set(previewer_SRCS plasma-previewer.cpp
                   previewitemmodel.cpp
                   previewdialog.cpp
                   previewwidget.cpp
                   )

QT4_ADD_DBUS_ADAPTOR(Previewer_SRCS org.kde.Previewer.xml
                     plasma-previewer.h Previewer)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_previewer ${previewer_SRCS} ${Previewer_SRCS})


target_link_libraries(plasma_applet_previewer
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KIO_LIBS} ${QT_QTDBUS_LIBRARY})
 

install(TARGETS plasma_applet_previewer
        DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-previewer.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

install(FILES preview.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus)

install(FILES
        previewer.svgz
        DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})

