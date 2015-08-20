# Builds the uisupport module

set(SOURCES
    abstractbuffercontainer.cpp
    abstractitemview.cpp
    action.cpp
    actioncollection.cpp
    bufferhotlistfilter.cpp
    bufferview.cpp
    bufferviewfilter.cpp
    bufferviewoverlayfilter.cpp
    clearablelineedit.cpp
    clickable.cpp
    clickablelabel.cpp
    colorbutton.cpp
    contextmenuactionprovider.cpp
    flatproxymodel.cpp
    fontselector.cpp
    graphicalui.cpp
    multilineedit.cpp
    networkmodelcontroller.cpp
    nickview.cpp
    nickviewfilter.cpp
    qssparser.cpp
    resizingstackedwidget.cpp
    settingspage.cpp
    styledlabel.cpp
    tabcompleter.cpp
    toolbaractionprovider.cpp
    uisettings.cpp
    uistyle.cpp

    # needed for automoc
    abstractnotificationbackend.h
)

include_directories(${KDE4_INCLUDES})
add_definitions(${KDE4_DEFINITIONS})
set(SOURCES ${SOURCES} kcmdlinewrapper.cpp)

include_directories(${PROJECT_SOURCE_DIR}/src/common
                    ${PROJECT_SOURCE_DIR}/src/client
)

add_library(mod_uisupport OBJECT ${SOURCES})

set(UISUPPORT_LIBRARIES
    ${UISUPPORT_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    PARENT_SCOPE
)
