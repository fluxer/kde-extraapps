
include_directories(
    ${KDevPlatform_SOURCE_DIR}
)

########### Plugin Selector
set( pluginsettings_cfg_SRCS
     pluginpreferences.cpp
)

kde4_add_plugin( kcm_kdev_pluginsettings ${pluginsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_pluginsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell )

########### UI settings
set( uisettings_cfg_SRCS
    uipreferences.cpp
    uiconfig.ui
)

kde4_add_kcfg_files( uisettings_cfg_SRCS uiconfig.kcfgc )
kde4_add_plugin( kcm_kdev_uisettings ${uisettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_uisettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime )

########### Background Parser

set( bgsettings_cfg_SRCS
    bgpreferences.cpp
    bgsettings.ui
)

kde4_add_kcfg_files( bgsettings_cfg_SRCS bgconfig.kcfgc )
kde4_add_plugin( kcm_kdev_bgsettings ${bgsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_bgsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformlanguage)

########### Background Parser

set( ccsettings_cfg_SRCS
    ccpreferences.cpp
    ccsettings.ui
)

kde4_add_kcfg_files( ccsettings_cfg_SRCS ccconfig.kcfgc )
kde4_add_plugin( kcm_kdev_ccsettings ${ccsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_ccsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatforminterfaces kdevplatformshell ${KDE4_KTEXTEDITOR_LIBS})

########### Project Configuration

set( projectsettings_cfg_SRCS
    projectpreferences.cpp
    projectsettings.ui
)

kde4_add_kcfg_files( projectsettings_cfg_SRCS projectconfig.kcfgc )
kde4_add_plugin( kcm_kdev_projectsettings ${projectsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_projectsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdevplatformshell)

########### Environment widget

set( envsettings_cfg_SRCS
  environmentwidget.cpp
  environmentgroupmodel.cpp
  environmentpreferences.cpp
  environmentwidget.ui
)

kde4_add_plugin( kcm_kdev_envsettings ${envsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_envsettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformutil)

########## Formatter Settings
set(kcm_kdevsourceformattersettings_SRCS
    editstyledialog.cpp
    sourceformattersettings.cpp
    editstyledialog.ui
    sourceformattersettings.ui
)

kde4_add_plugin(kcm_kdevsourceformattersettings ${kcm_kdevsourceformattersettings_SRCS})
target_link_libraries(kcm_kdevsourceformattersettings
    ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS}
    ${KDE4_KPARTS_LIBS} kdevplatformshell kdevplatformutil kdevplatformlanguage)

install(TARGETS  DESTINATION ${PLUGIN_INSTALL_DIR})


########## Install TARGETS/Files

install( TARGETS 
        kcm_kdev_projectsettings 
        kcm_kdev_ccsettings
        kcm_kdev_bgsettings
        kcm_kdev_pluginsettings
        kcm_kdev_envsettings 
        kcm_kdev_uisettings 
        kcm_kdevsourceformattersettings
        DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES 
        kcm_kdev_uisettings.desktop 
        kcm_kdev_envsettings.desktop
        kcm_kdev_pluginsettings.desktop
        kcm_kdev_projectsettings.desktop
        kcm_kdev_bgsettings.desktop
        kcm_kdev_ccsettings.desktop 
        kcm_kdevsourceformattersettings.desktop
        DESTINATION ${SERVICES_INSTALL_DIR} )
