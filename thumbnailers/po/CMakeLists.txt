project(kde-thumbnailer-po)

include_directories(
    ${GETTEXTPO_INCLUDE_DIR}
)

set(pothumbnail_SRCS pocreator.cpp)

kde4_add_kcfg_files(pothumbnail_SRCS pocreatorsettings.kcfgc)

kde4_add_plugin(pothumbnail ${pothumbnail_SRCS})
target_link_libraries(pothumbnail
    KDE4::kio
    ${GETTEXTPO_LIBRARY}
)

install(TARGETS pothumbnail DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES pothumbnail.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
