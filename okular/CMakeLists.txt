project(okular)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(GenerateExportHeader)
    include(FeatureSummary)

    find_package(KDELibs4 4.23.0 REQUIRED)

    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)

kde4_optional_find_package(Poppler)
set_package_properties(Poppler PROPERTIES
    DESCRIPTION "A PDF rendering library"
    URL "https://poppler.freedesktop.org"
    PURPOSE "Support for PDF files in Okular"
)

kde4_optional_find_package(LibSpectre 0.2)
set_package_properties(LibSpectre PROPERTIES
    DESCRIPTION "Small library for rendering Postscript documents"
    URL "https://www.freedesktop.org/wiki/Software/libspectre/"
    PURPOSE "Support for PS files in Okular"
)

kde4_optional_find_package(DjVuLibre 3.5.17)
set_package_properties(DjVuLibre PROPERTIES
    DESCRIPTION "Open source DjVu library"
    URL "https://djvu.sourceforge.net/"
    PURPOSE "Support for DjVu files in Okular"
)

kde4_optional_find_package(EPub)
set_package_properties(EPub PROPERTIES
    DESCRIPTION "A library for reading EPub documents"
    URL "http://sourceforge.net/projects/ebook-tools"
    PURPOSE "Support for EPub documents in Okular"
)

find_program(LATEX_AND_DVIPNG_EXECUTABLE latex dvipng)
add_feature_info(latex_and_dvipng
    LATEX_AND_DVIPNG_EXECUTABLE
    "LaTeX support in Okular"
)

option(
    OKULAR_FORCE_DRM
    "Forces okular to check for DRM to decide if you can copy/print protected pdf. (default=no)"
    OFF
)
kde4_bool_to_01(OKULAR_FORCE_DRM _OKULAR_FORCE_DRM)

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/config-okular.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-okular.h
)

add_subdirectory( ui )
add_subdirectory( shell )
add_subdirectory( generators )

if(ENABLE_TESTING)
    add_subdirectory( tests )
endif()

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
    core/utils.cpp
    core/view.cpp
    core/chooseenginewidget.ui
    core/textdocumentsettings.ui
)

install(
    FILES
    core/action.h
    core/annotations.h
    core/area.h
    core/document.h
    core/fontinfo.h
    core/form.h
    core/generator.h
    core/global.h
    core/page.h
    core/pagesize.h
    core/pagetransition.h
    core/sound.h
    core/sourcereference.h
    core/textdocumentgenerator.h
    core/textdocumentsettings.h
    core/textpage.h
    core/utils.h
    core/version.h
    core/observer.h
    ${CMAKE_CURRENT_BINARY_DIR}/settings_core.h
    ${CMAKE_CURRENT_BINARY_DIR}/okular_export.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/okular/core
)

install(
    FILES
    interfaces/configinterface.h
    interfaces/guiinterface.h
    interfaces/printinterface.h
    interfaces/saveinterface.h
    interfaces/viewerinterface.h
    DESTINATION ${KDE4_INCLUDE_INSTALL_DIR}/okular/interfaces
)

kde4_add_kcfg_files(okularcore_SRCS ${PROJECT_SOURCE_DIR}/conf/settings_core.kcfgc )

add_library(okularcore SHARED ${okularcore_SRCS})
target_link_libraries(okularcore
    KDE4::kio
    KDE4::karchive
    KDE4::kmediaplayer
    KDE4::solid
    ${MATH_LIB}
)

set_target_properties(okularcore PROPERTIES
    VERSION ${GENERIC_LIB_VERSION}
    SOVERSION ${GENERIC_LIB_SOVERSION}
)

generate_export_header(okularcore BASE_NAME okular)

install(
    TARGETS okularcore
    DESTINATION ${KDE4_LIB_INSTALL_DIR}
)

install(
    FILES core/okularGenerator.desktop
    DESTINATION ${KDE4_SERVICETYPES_INSTALL_DIR}
)

# okularpart

set(okularpart_SRCS
    part.cpp
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
    ui/videowidget.cpp
    conf/dlgaccessibilitybase.ui
    conf/dlgeditorbase.ui
    conf/dlggeneralbase.ui
    conf/dlgannotationsbase.ui
    conf/dlgperformancebase.ui
    conf/dlgpresentationbase.ui
)

kde4_add_kcfg_files(okularpart_SRCS ${PROJECT_SOURCE_DIR}/conf/settings.kcfgc )

kde4_add_plugin(okularpart ${okularpart_SRCS})

target_link_libraries(okularpart
    KDE4::kparts
    KDE4::kpasswdstore
    KDE4::karchive
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDBUS_LIBRARY}
    ${MATH_LIB}
    okularcore
)

generate_export_header(okularpart)

install(
    TARGETS okularpart
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES okular_part.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
install(
    FILES part.rc part-viewermode.rc
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/okular
)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
