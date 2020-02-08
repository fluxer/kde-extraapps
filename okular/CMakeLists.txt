project(okular)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${CMAKE_SOURCE_DIR}
)

add_subdirectory( plasma )
add_subdirectory( ui )
add_subdirectory( shell )
add_subdirectory( generators )

if(ENABLE_TESTING)
    add_subdirectory( tests )
endif()

include(OkularConfigureChecks.cmake)
include(MacroWriteBasicCMakeVersionFile)

set(MATH_LIB m)

# okularcore

set(okularcore_SRCS
   core/action.cpp
   core/annotations.cpp
   core/area.cpp
   core/audioplayer.cpp
   core/bookmarkmanager.cpp
   core/chooseenginedialog.cpp
   core/document.cpp
   core/documentcommands.cpp
   core/fontinfo.cpp
   core/form.cpp
   core/generator.cpp
   core/generator_p.cpp
   core/misc.cpp
   core/movie.cpp
   core/observer.cpp
   core/page.cpp
   core/pagecontroller.cpp
   core/pagesize.cpp
   core/pagetransition.cpp
   core/rotationjob.cpp
   core/sound.cpp
   core/sourcereference.cpp
   core/textdocumentgenerator.cpp
   core/textdocumentsettings.cpp
   core/textpage.cpp
   core/tilesmanager.cpp
   core/utils.cpp
   core/view.cpp
   core/fileprinter.cpp
   core/chooseenginewidget.ui
   core/textdocumentsettings.ui
)

install( FILES
           core/action.h
           core/annotations.h
           core/area.h
           core/document.h
           core/fontinfo.h
           core/form.h
           core/generator.h
           core/global.h
           core/okular_export.h
           core/page.h
           core/pagesize.h
           core/pagetransition.h
           core/sound.h
           core/sourcereference.h
           core/textdocumentgenerator.h
           core/textdocumentsettings.h
           core/textpage.h
           core/tile.h
           core/utils.h
           core/version.h
           core/fileprinter.h
           core/observer.h
           ${CMAKE_CURRENT_BINARY_DIR}/settings_core.h
         DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/okular/core COMPONENT Devel)

install( FILES
           interfaces/configinterface.h
           interfaces/guiinterface.h
           interfaces/printinterface.h
           interfaces/saveinterface.h
           interfaces/viewerinterface.h
         DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/okular/interfaces COMPONENT Devel)

kde4_add_kcfg_files(okularcore_SRCS ${PROJECT_SOURCE_DIR}/conf/settings_core.kcfgc )

add_library(okularcore SHARED ${okularcore_SRCS})

target_link_libraries(okularcore
    ${KDE4_KIO_LIBS}
    ${MATH_LIB}
    KDE4::kmediaplayer
)

set_target_properties(okularcore PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS okularcore ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES conf/okular.kcfg DESTINATION ${KDE4_KCFG_INSTALL_DIR})
install(FILES conf/okular_core.kcfg DESTINATION ${KDE4_KCFG_INSTALL_DIR})
install(FILES core/okularGenerator.desktop DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR})

# okularpart

set(okularpart_SRCS
   part.cpp
   extensions.cpp
   conf/preferencesdialog.cpp
   conf/dlgaccessibility.cpp
   conf/dlgdebug.cpp
   conf/dlgeditor.cpp
   conf/dlggeneral.cpp
   conf/dlgannotations.cpp
   conf/dlgperformance.cpp
   conf/dlgpresentation.cpp
   conf/widgetannottools.cpp
   ui/embeddedfilesdialog.cpp
   ui/annotwindow.cpp
   ui/annotationmodel.cpp
   ui/annotationpopup.cpp
   ui/annotationpropertiesdialog.cpp
   ui/annotationproxymodels.cpp
   ui/annotationtools.cpp
   ui/annotationwidgets.cpp
   ui/bookmarklist.cpp
   ui/fileprinterpreview.cpp
   ui/findbar.cpp
   ui/formwidgets.cpp
   ui/guiutils.cpp
   ui/ktreeviewsearchline.cpp
   ui/latexrenderer.cpp
   ui/minibar.cpp
   ui/pageitemdelegate.cpp
   ui/pagepainter.cpp
   ui/pagesizelabel.cpp
   ui/pageviewannotator.cpp
   ui/pageview.cpp
   ui/magnifierview.cpp
   ui/pageviewutils.cpp
   ui/presentationsearchbar.cpp
   ui/presentationwidget.cpp
   ui/propertiesdialog.cpp
   ui/searchlineedit.cpp
   ui/searchwidget.cpp
   ui/sidebar.cpp
   ui/side_reviews.cpp
   ui/snapshottaker.cpp
   ui/thumbnaillist.cpp
   ui/toc.cpp
   ui/tocmodel.cpp
   ui/toolaction.cpp
   ui/tts.cpp
   ui/videowidget.cpp
   conf/dlgaccessibilitybase.ui
   conf/dlgeditorbase.ui
   conf/dlggeneralbase.ui
   conf/dlgannotationsbase.ui
   conf/dlgperformancebase.ui
   conf/dlgpresentationbase.ui
)

kde4_add_kcfg_files(okularpart_SRCS ${PROJECT_SOURCE_DIR}/conf/settings.kcfgc )

qt4_add_dbus_interfaces(okularpart_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml)

kde4_add_plugin(okularpart SHARED ${okularpart_SRCS})

target_link_libraries(okularpart okularcore ${KDE4_KPARTS_LIBS} ${KDE4_KPRINTUTILS_LIBS} ${MATH_LIB} ${KDE4_SOLID_LIBRARY})

install(TARGETS okularpart DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES okular_part.desktop  DESTINATION  ${KDE4_SERVICES_INSTALL_DIR} )
install( FILES part.rc part-viewermode.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/okular )

########### cmake files #################

macro_write_basic_cmake_version_file(${CMAKE_CURRENT_BINARY_DIR}/OkularConfigVersion.cmake 0 11 80)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/OkularConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/OkularConfigVersion.cmake
        DESTINATION ${KDE4_LIB_INSTALL_DIR}/cmake/Okular )

