project(importer)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(importer_SRCS
    importerconfigdialog.cpp
    dialogpage.cpp
    documentdirfinder.cpp
    fileutils.cpp
    main.cpp
    importdialog.cpp
    importer.cpp
    progresspage.cpp
    filenameformater.cpp
    serializedurlmap.cpp
    thumbnailpage.cpp
    dialogpage.ui
    importerconfigdialog.ui
    progresspage.ui
    thumbnailpage.ui
)

kde4_add_kcfg_files(importer_SRCS importerconfig.kcfgc)

add_executable(gwenview_importer ${importer_SRCS})
target_link_libraries(gwenview_importer
    KDE4::kfile
    KDE4::kio
    KDE4::solid
    ${QT_QTCORE_LIBRARY}
    gwenviewlib
)

install(
    TARGETS gwenview_importer
    DESTINATION ${KDE4_BIN_INSTALL_DIR}
)

install(FILES gwenview_importer.desktop
    gwenview_importer_camera.desktop
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/solid/actions/)
