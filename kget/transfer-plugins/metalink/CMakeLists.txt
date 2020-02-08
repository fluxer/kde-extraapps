#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/metalink+xml;" PARENT_SCOPE)

include_directories(
   ../../
)

set(kget_metalinkfactory_PART_SRCS
    metalinkfactory.cpp
    abstractmetalink.cpp
    metalinkxml.cpp
    metalinkhttp.cpp
    fileselectiondlg.cpp
    ../../ui/metalinkcreator/metalinker.cpp
    fileselection.ui
)

kde4_add_kcfg_files(kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)

kde4_add_plugin(kget_metalinkfactory ${kget_metalinkfactory_PART_SRCS})

target_link_libraries(kget_metalinkfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kgetcore)

install(TARGETS kget_metalinkfactory DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES kget_metalinkfactory.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})


###Build KCM-Module
set(kcm_kget_metalinkfactory_PART_SRCS
    dlgmetalink.cpp
    dlgmetalink.ui
)

kde4_add_kcfg_files(kcm_kget_metalinkfactory_PART_SRCS metalinksettings.kcfgc)
kde4_add_plugin(kcm_kget_metalinkfactory ${kcm_kget_metalinkfactory_PART_SRCS})
target_link_libraries(kcm_kget_metalinkfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_metalinkfactory DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES kget_metalinkfactory_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
