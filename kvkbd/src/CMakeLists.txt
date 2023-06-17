include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})

set(kvkbd_SRCS vkeyboard.cpp 
    x11keyboard.cpp
    dragwidget.cpp
    mainwidget.cpp
    vbutton.cpp
    main.cpp
    resizabledragwidget.cpp
    keysymconvert.cpp
    kbddock.cpp
    kvkbdapp.cpp
    kbdtray.cpp
    themeloader.cpp
)

QT4_ADD_DBUS_ADAPTOR(kvkbd_SRCS org.kde.kvkbd.Kvkbd.xml
                       mainwidget.h MainWidget)

QT4_ADD_DBUS_ADAPTOR(kvkbd_SRCS org.kde.kvkbd.Dock.xml
                       kbddock.h KbdDock)

add_executable(kvkbd ${kvkbd_SRCS})

target_link_libraries(kvkbd
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${QT_QTDBUS_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${X11_X11_LIB}
    ${X11_XTest_LIB}
)

install(
    TARGETS kvkbd
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

install(
    FILES kvkbd.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
