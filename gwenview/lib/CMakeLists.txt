project(gwenviewlib)


include_directories(
    # For config-gwenview.h
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}
    ${EXIV2_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
    ${PNG_INCLUDE_DIRS}
)

set(gwenviewlib_SRCS
    contextmanager.cpp
    crop/cropwidget.cpp
    crop/cropimageoperation.cpp
    crop/croptool.cpp
    document/abstractdocumentimpl.cpp
    document/documentjob.cpp
    document/animateddocumentloadedimpl.cpp
    document/document.cpp
    document/documentfactory.cpp
    document/documentloadedimpl.cpp
    document/emptydocumentimpl.cpp
    document/loadingdocumentimpl.cpp
    document/loadingjob.cpp
    document/savejob.cpp
    document/svgdocumentloadedimpl.cpp
    documentview/abstractdocumentviewadapter.cpp
    documentview/abstractimageview.cpp
    documentview/abstractrasterimageviewtool.cpp
    documentview/birdeyeview.cpp
    documentview/documentview.cpp
    documentview/documentviewcontroller.cpp
    documentview/documentviewsynchronizer.cpp
    documentview/loadingindicator.cpp
    documentview/messageviewadapter.cpp
    documentview/rasterimageview.cpp
    documentview/rasterimageviewadapter.cpp
    documentview/svgviewadapter.cpp
    about.cpp
    abstractimageoperation.cpp
    disabledactionshortcutmonitor.cpp
    documentonlyproxymodel.cpp
    documentview/documentviewcontainer.cpp
    binder.cpp
    eventwatcher.cpp
    historymodel.cpp
    archiveutils.cpp
    datewidget.cpp
    exiv2imageloader.cpp
    flowlayout.cpp
    fullscreenbar.cpp
    hud/hudbutton.cpp
    hud/hudbuttonbox.cpp
    hud/hudcountdown.cpp
    hud/hudlabel.cpp
    hud/hudmessagebubble.cpp
    hud/hudslider.cpp
    hud/hudtheme.cpp
    hud/hudwidget.cpp
    graphicswidgetfloater.cpp
    imagemetainfomodel.cpp
    imagescaler.cpp
    imageutils.cpp
    invisiblebuttongroup.cpp
    kindproxymodel.cpp
    memoryutils.cpp
    mimetypeutils.cpp
    paintutils.cpp
    placetreemodel.cpp
    preferredimagemetainfomodel.cpp
    print/printhelper.cpp
    print/printoptionspage.cpp
    recursivedirmodel.cpp
    shadowfilter.cpp
    slidecontainer.cpp
    slideshow.cpp
    statusbartoolbutton.cpp
    redeyereduction/redeyereductionimageoperation.cpp
    redeyereduction/redeyereductiontool.cpp
    resize/resizeimageoperation.cpp
    resize/resizeimagedialog.cpp
    thumbnailprovider/thumbnailgenerator.cpp
    thumbnailprovider/thumbnailprovider.cpp
    thumbnailprovider/thumbnailwriter.cpp
    thumbnailview/abstractthumbnailviewhelper.cpp
    thumbnailview/abstractdocumentinfoprovider.cpp
    thumbnailview/contextbarbutton.cpp
    thumbnailview/dragpixmapgenerator.cpp
    thumbnailview/itemeditor.cpp
    thumbnailview/previewitemdelegate.cpp
    thumbnailview/thumbnailbarview.cpp
    thumbnailview/thumbnailslider.cpp
    thumbnailview/thumbnailview.cpp
    thumbnailview/tooltipwidget.cpp
    timeutils.cpp
    transformimageoperation.cpp
    urlutils.cpp
    widgetfloater.cpp
    zoomslider.cpp
    zoomwidget.cpp
    sorteddirmodel.cpp
)

set_source_files_properties(
    exiv2imageloader.cpp
    imagemetainfomodel.cpp
    timeutils.cpp
    PROPERTIES
    COMPILE_FLAGS "${KDE4_ENABLE_EXCEPTIONS}"
)

kde4_add_kcfg_files(gwenviewlib_SRCS gwenviewconfig.kcfgc)

add_library(gwenviewlib SHARED ${gwenviewlib_SRCS})
set_target_properties(gwenviewlib PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(gwenviewlib
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PHONON_LIBS}
    ${KDE4_KDCRAW_LIBS}
    ${JPEG_LIBRARY}
    ${EXIV2_LIBRARIES}
    ${QT_QTOPENGL_LIBRARY}
    ${X11_X11_LIB}
    ${PNG_LIBRARIES}
)

install(TARGETS gwenviewlib ${INSTALL_TARGETS_DEFAULT_ARGS})
