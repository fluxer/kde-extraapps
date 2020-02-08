#Set supported mime type
SET(SUPPORTED_KGET_MIMETYPES "${SUPPORTED_KGET_MIMETYPES}application/x-bittorrent;" PARENT_SCOPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}")

include_directories(
    ../../
    ${CMAKE_SOURCE_DIR}/libktorrent/src
)
set(kget_bittorrentfactory_PART_SRCS
    bttransfer.cpp
    bttransferfactory.cpp
    bttransferhandler.cpp
    btdetailswidget.cpp
    scandlg.cpp
    #btdatasource.cpp
    #btchunkselector.cpp
    #btcache.cpp
    advanceddetails/btadvanceddetailswidget.cpp
    advanceddetails/chunkdownloadmodel.cpp
    advanceddetails/chunkdownloadview.cpp
    advanceddetails/fileview.cpp
    advanceddetails/iwfilelistmodel.cpp
    advanceddetails/iwfiletreemodel.cpp
    advanceddetails/peerview.cpp
    advanceddetails/peerviewmodel.cpp
    advanceddetails/torrentfilelistmodel.cpp
    advanceddetails/torrentfilemodel.cpp
    advanceddetails/torrentfiletreemodel.cpp
    advanceddetails/monitor.cpp
    advanceddetails/trackerview.cpp
    advanceddetails/trackermodel.cpp
    advanceddetails/webseedsmodel.cpp
    advanceddetails/webseedstab.cpp
    btdetailswidgetfrm.ui
    scandlg.ui
    advanceddetails/chunkdownloadview.ui
    advanceddetails/trackerview.ui
    advanceddetails/webseedstab.ui
)

kde4_add_kcfg_files(kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)
kde4_add_plugin(kget_bittorrentfactory ${kget_bittorrentfactory_PART_SRCS})

target_link_libraries(kget_bittorrentfactory ${KDE4_KIO_LIBS} ktorrent kgetcore)
install(TARGETS kget_bittorrentfactory DESTINATION  ${KDE4_PLUGIN_INSTALL_DIR})
install(FILES kget_bittorrentfactory.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})

###Build KCM-Module
set(kcm_kget_bittorrentfactory_PART_SRCS
    btsettingswidget.cpp
    btsettingswidget.ui
)

kde4_add_kcfg_files(kcm_kget_bittorrentfactory_PART_SRCS bittorrentsettings.kcfgc)
kde4_add_plugin(kcm_kget_bittorrentfactory ${kcm_kget_bittorrentfactory_PART_SRCS})
target_link_libraries(kcm_kget_bittorrentfactory ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS})
install(TARGETS kcm_kget_bittorrentfactory DESTINATION ${KDE4_PLUGIN_INSTALL_DIR})

install(FILES kget_bittorrentfactory_config.desktop DESTINATION ${KDE4_SERVICES_INSTALL_DIR})
