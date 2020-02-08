project(ksnapshot)

macro_optional_find_package(Kipi)

add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
include_directories (${QDBUS_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

set(HAVE_X11_EXTENSIONS_SHAPE_H ${X11_Xshape_FOUND})
set(HAVE_X11_EXTENSIONS_XFIXES_H ${X11_Xfixes_FOUND})
macro_log_feature(HAVE_X11_EXTENSIONS_XFIXES_H "X11 Fixes Extension (xfixes.h)" "Support for capturing the cursor" "http://www.x.org/" FALSE "" "")

if (KIPI_FOUND)
        include_directories(${KIPI_INCLUDE_DIR})
endif (KIPI_FOUND)
macro_log_feature(KIPI_FOUND "KIPI plugins" "Provides various image export features, such as printing, emailing and uploading" "http://www.kipi-plugins.org/" FALSE "" "")

configure_file(config-ksnapshot.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-ksnapshot.h)

########### next target ###############

set(ksnapshot_file_SRCS
   expblur.cpp
	regiongrabber.cpp
        freeregiongrabber.cpp
	snapshottimer.cpp
	windowgrabber.cpp
	ksnapshotobject.cpp
	ksnapshotpreview.cpp)

set(ksnapshot_SRCS
   main.cpp
   ksnapshot.cpp
   ksnapshotwidget.ui
   ${ksnapshot_file_SRCS})

if (KIPI_FOUND)
set(ksnapshot_SRCS
    kipiinterface.cpp
    ksnapshotimagecollectionshared.cpp
    ksnapshotinfoshared.cpp
    kipiimagecollectionselector.cpp
    ${ksnapshot_SRCS})
endif (KIPI_FOUND)

qt4_add_dbus_adaptor(ksnapshot_SRCS org.kde.ksnapshot.xml ksnapshot.h KSnapshot)

add_executable(ksnapshot ${ksnapshot_SRCS})

target_link_libraries(ksnapshot ${KDE4_KIO_LIBS} ${X11_LIBRARIES})

if (X11_Xfixes_FOUND)
    target_link_libraries(ksnapshot ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)

if (KIPI_FOUND)
    target_link_libraries(ksnapshot ${KIPI_LIBRARIES})
endif (KIPI_FOUND)

install(TARGETS ksnapshot ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kbackgroundsnapshot_SRCS
   kbackgroundsnapshot.cpp
   ${ksnapshot_file_SRCS}
)

add_executable(kbackgroundsnapshot ${kbackgroundsnapshot_SRCS})

target_link_libraries(kbackgroundsnapshot ${KDE4_KIO_LIBS} ${X11_LIBRARIES})

install(TARGETS kbackgroundsnapshot ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############



install(PROGRAMS ksnapshot.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES org.kde.ksnapshot.xml DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR} )
kde4_install_icons(${KDE4_ICON_INSTALL_DIR} )
