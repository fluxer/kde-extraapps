project(rawthumbnail)

## includes and linking
include_directories(${KEXIV2_INCLUDE_DIR} ${KDCRAW_INCLUDE_DIR})


#sources
set(rawthumbnail_SRCS rawcreator.cpp)
kde4_add_plugin(rawthumbnail ${rawthumbnail_SRCS})


## includes and linking
target_link_libraries(rawthumbnail
    ${KDE4_KIO_LIBS}
    ${KDE4_KDCRAW_LIBS}
    ${KDE4_KEXIV2_LIBS}
    ${KDE4_KDECORE_LIBS}
    ${QT_QTGUI_LIBRARY}
)

## install the plugin
install(TARGETS rawthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES rawthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})


