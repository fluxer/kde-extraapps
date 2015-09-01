project(importer)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
    ${EXIV2_INCLUDE_DIR}
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

target_link_libraries(gwenview_importer PRIVATE
    ${KDE4_KFILE_LIBS}
    gwenviewlib
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QT_QTCORE_LIBRARY}
)

target_link_libraries(gwenview_importer PUBLIC
    ${KDE4_KIO_LIBS}
    ${KDE4_SOLID_LIBS}
    ${QT_QTCORE_LIBRARY}
)

install(TARGETS gwenview_importer ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES gwenview_importer.desktop
    gwenview_importer_camera.desktop
    DESTINATION ${DATA_INSTALL_DIR}/solid/actions/)
