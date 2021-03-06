project(ksnapshot)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

macro_optional_find_package(Kipi)
set_package_properties(Kipi PROPERTIES
    URL "https://github.com/KDE/kipi-plugins"
    PURPOSE "Provides various image manipulation and export features"
)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

set(HAVE_X11_EXTENSIONS_SHAPE_H ${X11_Xshape_FOUND})
add_feature_info(
    "Support for capturing border"
    HAVE_X11_EXTENSIONS_SHAPE_H
    "X11 Shape Extension (shape.h)"
)

set(HAVE_X11_EXTENSIONS_XFIXES_H ${X11_Xfixes_FOUND})
add_feature_info(
    "Support for capturing the cursor"
    HAVE_X11_EXTENSIONS_XFIXES_H
    "X11 Fixes Extension (xfixes.h)"
)

if (KIPI_FOUND)
    include_directories(${KIPI_INCLUDE_DIR})
endif()

configure_file(
    config-ksnapshot.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/config-ksnapshot.h
)

########### next target ###############

set(ksnapshot_file_SRCS
    expblur.cpp
    regiongrabber.cpp
    freeregiongrabber.cpp
    snapshottimer.cpp
    windowgrabber.cpp
    ksnapshotobject.cpp
    ksnapshotpreview.cpp
)

set(ksnapshot_SRCS
    main.cpp
    ksnapshot.cpp
    ksnapshotwidget.ui
    ${ksnapshot_file_SRCS}
)

if (KIPI_FOUND)
    set(ksnapshot_SRCS
        kipiinterface.cpp
        ksnapshotimagecollectionshared.cpp
        ksnapshotinfoshared.cpp
        kipiimagecollectionselector.cpp
        ${ksnapshot_SRCS}
    )
endif()

qt4_add_dbus_adaptor(ksnapshot_SRCS org.kde.ksnapshot.xml ksnapshot.h KSnapshot)

add_executable(ksnapshot ${ksnapshot_SRCS})

target_link_libraries(ksnapshot ${KDE4_KIO_LIBS} ${X11_LIBRARIES})

if (X11_Xfixes_FOUND)
    target_link_libraries(ksnapshot ${X11_Xfixes_LIB})
endif (X11_Xfixes_FOUND)

if (KIPI_FOUND)
    target_link_libraries(ksnapshot ${KIPI_LIBRARIES})
endif()

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
