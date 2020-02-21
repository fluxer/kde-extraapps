# Project Needs a name ofcourse
project(plasma-knowledgebase)
 
 
# We add our source code here
set(knowledgebase_SRCS
    kbitemwidget.cpp
    knowledgebase.cpp
    kbitemtitle.cpp
    knowledgeBaseConfig.ui
)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_knowledgebase ${knowledgebase_SRCS})
target_link_libraries(plasma_applet_knowledgebase 
    ${KDE4_PLASMA_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)
 
install(
    TARGETS plasma_applet_knowledgebase
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)
 
install(
    FILES plasma-applet-knowledgebase.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
