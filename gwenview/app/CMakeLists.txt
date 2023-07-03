include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    # For lib/gwenviewconfig.h
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(gwenview_SRCS
    abstractcontextmanageritem.cpp
    configdialog.cpp
    gvcore.cpp
    documentinfoprovider.cpp
    viewmainpage.cpp
    fileoperations.cpp
    filtercontroller.cpp
    fullscreencontent.cpp
    infocontextmanageritem.cpp
    imagemetainfodialog.cpp
    imageopscontextmanageritem.cpp
    fileopscontextmanageritem.cpp
    main.cpp
    mainwindow.cpp
    preloader.cpp
    saveallhelper.cpp
    savebar.cpp
    sidebar.cpp
    startmainpage.cpp
    thumbnailviewhelper.cpp
    browsemainpage.cpp
    advancedconfigpage.ui
    fullscreenconfigwidget.ui
    generalconfigpage.ui
    imageviewconfigpage.ui
    startmainpage.ui
    browsemainpage.ui
)

add_executable(gwenview ${gwenview_SRCS})

target_link_libraries(gwenview
    KDE4::kfile
    KDE4::kio
    KDE4Workspace::konq
    ${QT_QTCORE_LIBRARY}
    gwenviewlib
)

install(TARGETS gwenview DESTINATION ${KDE4_BIN_INSTALL_DIR})

install(FILES gwenviewui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/gwenview)

install(PROGRAMS gwenview.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

install(FILES slideshow.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus)
