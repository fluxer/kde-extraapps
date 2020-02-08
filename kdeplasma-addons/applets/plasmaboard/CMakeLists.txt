# Project Needs a name ofcourse
project(plasmaboard)

# We add our source code here
set(plasmaboard_SRCS
    widget.cpp
    BoardKey.cpp
    PanelIcon.cpp
    AlphaNumKey.cpp
    DualKey.cpp
    FuncKey.cpp
    EnterKey.cpp
    BackspaceKey.cpp
    TabKey.cpp
    ShiftKey.cpp
    CapsKey.cpp
    ArrowTopKey.cpp
    ArrowBottomKey.cpp
    ArrowLeftKey.cpp
    ArrowRightKey.cpp
    tooltip.cpp
    StickyKey.cpp
    SwitchKey.cpp
    Layout.cpp
    config.ui
)

if (Q_WS_X11)
    set(plasmaboard_SRCS ${plasmaboard_SRCS} Helpers_x11.cpp)
elseif (Q_WS_X11)
    set(plasmaboard_SRCS ${plasmaboard_SRCS} Helpers_fake.cpp)
endif (Q_WS_X11)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_plasmaboard ${plasmaboard_SRCS})
target_link_libraries(plasma_applet_plasmaboard 
                ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${X11_XTest_LIB} ${X11_X11_LIB} ${X11_LIBS})
 
install(TARGETS plasma_applet_plasmaboard DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES plasma_applet_plasmaboard.desktop
        DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

add_subdirectory(layouts)
        
#install(FILES plasmaboard_key.svg
#	DESTINATION ${KDE4_DATA_INSTALL_DIR}/desktoptheme/default/widgets/)
