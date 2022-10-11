########### next target ###############

include_directories(
   ${POPPLER_INCLUDE_DIR}
)

set(pdfthumbnail_PART_SRCS pdfcreator.cpp)

kde4_add_kcfg_files(pdfthumbnail_PART_SRCS pdfcreatorsettings.kcfgc)
kde4_add_plugin(pdfthumbnail ${pdfthumbnail_PART_SRCS})

target_link_libraries(pdfthumbnail
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${POPPLER_LIBRARIES}
)

install(
    TARGETS pdfthumbnail
    DESTINATION ${KDE4_PLUGIN_INSTALL_DIR}
)

########### install files ###############

install(
    FILES pdfthumbnail.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}
)
