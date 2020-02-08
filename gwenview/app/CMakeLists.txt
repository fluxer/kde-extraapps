include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${EXIV2_INCLUDE_DIR}
    ${LIBKONQ_INCLUDE_DIR}
    # For lib/gwenviewconfig.h and config-gwenview.h
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

if(KIPI_FOUND)
    include_directories(${KIPI_INCLUDE_DIR})
endif()

set(gwenview_SRCS
    abstractcontextmanageritem.cpp
    configdialog.cpp
    gvcore.cpp
    documentinfoprovider.cpp
    viewmainpage.cpp
    fileoperations.cpp
    filtercontroller.cpp
    folderviewcontextmanageritem.cpp
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

if(KIPI_FOUND)
    set(gwenview_SRCS
        ${gwenview_SRCS}
        kipiexportaction.cpp
        kipiimagecollectionselector.cpp
        kipiinterface.cpp
        kipiuploadwidget.cpp
    )
endif()

add_executable(gwenview ${gwenview_SRCS})

target_link_libraries(gwenview
    ${KDE4_KFILE_LIBS}
    gwenviewlib
    ${KDE4_KIO_LIBS}
    ${LIBKONQ_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KIO_LIBS}
)

if(KIPI_FOUND)
    target_link_libraries(gwenview ${KIPI_LIBRARIES})
endif()

install(TARGETS gwenview ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES gwenviewui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/gwenview)

install(PROGRAMS gwenview.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})

install(FILES slideshow.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus)
