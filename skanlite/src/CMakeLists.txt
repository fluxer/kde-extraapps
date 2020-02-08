include_directories(${CMAKE_SOURCE_DIR}/libksane)

set(skanlite_SRCS
    main.cpp
    skanlite.cpp
    ImageViewer.cpp
    KSaneImageSaver.cpp
    SaveLocation.cpp
    settings.ui
    SaveLocation.ui
)

add_executable(skanlite ${skanlite_SRCS})

target_link_libraries(skanlite ${KDE4_KIO_LIBS} ${PNG_LIBRARY} ksane)

install(TARGETS skanlite ${INSTALL_TARGETS_DEFAULT_ARGS})
install( PROGRAMS skanlite.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR} )
