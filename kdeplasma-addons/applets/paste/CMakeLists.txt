project(plasma-paste)

set(paste_SRCS
    paste.cpp
    sendkeys.cpp
    list.cpp
    configdata.cpp
    snippetconfig.cpp
    autopasteconfig.cpp
    appkey.cpp
    pastemacroexpander.cpp
    addmacro.cpp
    list.ui
    snippetconfig.ui
    autopasteconfig.ui
    appkey.ui
)

kde4_add_plugin(plasma_applet_paste ${paste_SRCS})
target_link_libraries(plasma_applet_paste
        ${X11_X11_LIB} KDE4::kdeui KDE4::kio KDE4::plasma)

install(TARGETS plasma_applet_paste DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-paste.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
