# Builds the qtui module

set(SOURCES
    awaylogfilter.cpp
    awaylogview.cpp
    bufferwidget.cpp
    channellistdlg.cpp
    chatitem.cpp
    chatline.cpp
    chatlinemodel.cpp
    chatlinemodelitem.cpp
    chatmonitorfilter.cpp
    chatmonitorview.cpp
    chatscene.cpp
    chatview.cpp
    chatviewsearchbar.cpp
    chatviewsearchcontroller.cpp
    chatviewsettings.cpp
    columnhandleitem.cpp
    coreconfigwizard.cpp
    coreconnectdlg.cpp
    coreconnectionstatuswidget.cpp
    coreinfodlg.cpp
    debugbufferviewoverlay.cpp
    debugconsole.cpp
    debuglogwidget.cpp
    debugmessagemodelfilter.cpp
    inputwidget.cpp
    ircconnectionwizard.cpp
    legacysystemtray.cpp
    mainpage.cpp
    mainwin.cpp
    markerlineitem.cpp
    msgprocessorstatuswidget.cpp
    nicklistwidget.cpp
    qtui.cpp
    qtuiapplication.cpp
    qtuimessageprocessor.cpp
    qtuisettings.cpp
    qtuistyle.cpp
    receivefiledlg.cpp
    settingsdlg.cpp
    settingspagedlg.cpp
    simplenetworkeditor.cpp
    systemtray.cpp
    systraynotificationbackend.cpp
    taskbarnotificationbackend.cpp
    titlesetter.cpp
    topicwidget.cpp
    verticaldock.cpp
)

set(FORMS
    bufferviewwidget.ui
    bufferwidget.ui
    channellistdlg.ui
    chatviewsearchbar.ui
    coreconfigwizardintropage.ui
    coreconfigwizardadminuserpage.ui
    coreconfigwizardstorageselectionpage.ui
    coreconfigwizardsyncpage.ui
    coreconnectauthdlg.ui
    coreconnectionstatuswidget.ui
    coreinfodlg.ui
    debugbufferviewoverlay.ui
    debugconsole.ui
    debuglogwidget.ui
    inputwidget.ui
    msgprocessorstatuswidget.ui
    nicklistwidget.ui
    settingsdlg.ui
    settingspagedlg.ui
    simplenetworkeditor.ui
    receivefiledlg.ui
    topicwidget.ui
)

set(LIBS )
set(QT_MODULES )

add_definitions(${KDE4_DEFINITIONS})
include_directories(${KDE4_INCLUDES})
list(APPEND SOURCES knotificationbackend.cpp)
list(APPEND LIBS ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBRARY} ${KDE4_KNOTIFYCONFIG_LIBRARY})

if (QT_QTDBUS_FOUND)
    add_definitions(-DHAVE_DBUS)
    list(APPEND QT_MODULES DBus)
    if (dbusmenu-qt_FOUND)
        add_definitions(-DHAVE_DBUSMENU)
        include_directories(${dbusmenu-qt_INCLUDE_DIRS} ${dbusmenu-qt5_INCLUDE_DIRS})
        list(APPEND LIBS dbusmenu-qt)
    endif()

    list(APPEND SOURCES statusnotifieritem.cpp statusnotifieritemdbus.cpp)
    qt4_add_dbus_interface(SOURCES ../../interfaces/org.kde.StatusNotifierWatcher.xml statusnotifierwatcher)
    qt4_add_dbus_interface(SOURCES ../../interfaces/org.freedesktop.Notifications.xml notificationsclient)
    qt4_add_dbus_adaptor  (SOURCES ../../interfaces/org.kde.StatusNotifierItem.xml statusnotifieritemdbus.h StatusNotifierItemDBus)
endif()

if(HAVE_SSL)
  set(SOURCES ${SOURCES} sslinfodlg.cpp)
  set(FORMS ${FORMS} sslinfodlg.ui)
endif(HAVE_SSL)

if (INDICATEQT_FOUND)
    add_definitions(-DHAVE_INDICATEQT)
    include_directories(${INDICATEQT_INCLUDE_DIRS})
    list(APPEND SOURCES indicatornotificationbackend.cpp)
    list(APPEND FORMS   indicatornotificationconfigwidget.ui)
    list(APPEND LIBS ${INDICATEQT_LIBRARIES})
endif()

foreach(FORM ${FORMS})
  set(FORMPATH ${FORMPATH} ui/${FORM})
endforeach(FORM ${FORMS})

# handle settingspages
include(settingspages/settingspages.cmake)
foreach(SP ${SETTINGSPAGES})
  set(SPSRC ${SPSRC} settingspages/${SP}settingspage.cpp)
  set(SPFRM ${SPFRM} settingspages/${SP}settingspage.ui)
endforeach(SP ${SETTINGSPAGES})
foreach(SRC ${SP_SOURCES})
  set(SPSRC ${SPSRC} settingspages/${SRC})
endforeach(SRC ${SP_SOURCES})
foreach(FRM ${SP_FORMS})
  set(SPFRM ${SPFRM} settingspages/${FRM})
endforeach(FRM ${SP_FORMS})

include_directories(${PROJECT_SOURCE_DIR}/src/common
                    ${PROJECT_SOURCE_DIR}/src/client
                    ${PROJECT_SOURCE_DIR}/src/qtui
                    ${PROJECT_SOURCE_DIR}/src/qtui/settingspages
                    ${PROJECT_SOURCE_DIR}/src/uisupport

)

qt4_wrap_ui(UI ${FORMPATH} ${SPFRM})


add_library(mod_qtui STATIC ${SOURCES} ${SPSRC} ${UI})

target_link_libraries(mod_qtui
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${LIBS}
)

add_dependencies(mod_qtui mod_common mod_client mod_uisupport)
