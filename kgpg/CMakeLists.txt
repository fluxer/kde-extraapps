project(kgpg)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    find_package(KDE4 4.19.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

# only headers are used
find_package( Gpgme REQUIRED )

add_subdirectory(icons)
include_directories(${GPGME_INCLUDES})

option(KGPG_DEBUG_TRANSACTIONS "show commands and results of gpg calls in debug log" Off)

set(core_SRCS
    core/convert.cpp
    core/images.cpp
    core/emailvalidator.cpp
    core/kgpgkey.cpp
    core/KGpgExpandableNode.cpp
    core/KGpgKeyNode.cpp
    core/KGpgGroupMemberNode.cpp
    core/KGpgGroupNode.cpp
    core/KGpgNode.cpp
    core/KGpgOrphanNode.cpp
    core/KGpgRefNode.cpp
    core/KGpgRootNode.cpp
    core/KGpgSignableNode.cpp
    core/KGpgSignNode.cpp
    core/KGpgSubkeyNode.cpp
    core/KGpgUatNode.cpp
    core/KGpgUidNode.cpp
)

set(kgpg_editor_SRCS
    editor/kgpgmd5widget.cpp
    editor/kgpgeditor.cpp
    editor/kgpgtextedit.cpp
)

set(kgpg_model_SRCS
    model/gpgservermodel.cpp
    model/kgpgitemmodel.cpp
    model/groupeditproxymodel.cpp
    model/keylistproxymodel.cpp
    model/kgpgsearchresultmodel.cpp
    model/selectkeyproxymodel.cpp
)

set(kgpg_transaction_SRCS
    transactions/kgpgtransaction.cpp
    transactions/kgpgtransactionjob.cpp
    transactions/kgpguidtransaction.cpp
    transactions/kgpgeditkeytransaction.cpp
    transactions/kgpgsigntransactionhelper.cpp
    transactions/kgpgchangedisable.cpp
    transactions/kgpgchangeexpire.cpp
    transactions/kgpgchangepass.cpp
    transactions/kgpgchangetrust.cpp
    transactions/kgpgdelsign.cpp
    transactions/kgpgdeluid.cpp
    transactions/kgpgencrypt.cpp
    transactions/kgpgexport.cpp
    transactions/kgpgaddphoto.cpp
    transactions/kgpgadduid.cpp
    transactions/kgpggeneratekey.cpp
    transactions/kgpggeneraterevoke.cpp
    transactions/kgpgdecrypt.cpp
    transactions/kgpgdelkey.cpp
    transactions/kgpgimport.cpp
    transactions/kgpgkeyservertransaction.cpp
    transactions/kgpgkeyservergettransaction.cpp
    transactions/kgpgkeyserversearchtransaction.cpp
    transactions/kgpgprimaryuid.cpp
    transactions/kgpgsendkeys.cpp
    transactions/kgpgsignkey.cpp
    transactions/kgpgsigntext.cpp
    transactions/kgpgsignuid.cpp
    transactions/kgpgtextorfiletransaction.cpp
    transactions/kgpgverify.cpp
)

set(kgpg_SRCS
    ${core_SRCS}
    ${kgpg_editor_SRCS}
    ${kgpg_model_SRCS}
    ${kgpg_transaction_SRCS}
    selectpublickeydialog.cpp
    selectsecretkey.cpp
    kgpgoptions.cpp
    keysmanager.cpp
    kgpg.cpp
    main.cpp
    kgpgkeygenerate.cpp
    kgpginterface.cpp
    kgpgtextinterface.cpp
    gpgproc.cpp
    klinebufferedprocess.cpp
    keyservers.cpp
    detailedconsole.cpp
    keyinfodialog.cpp
    newkey.cpp
    keyexport.cpp
    conf_encryption.cpp
    kgpgrevokewidget.cpp
    keytreeview.cpp
    groupedit.cpp
    sourceselect.cpp
    kgpgchangekey.cpp
    kgpgfirstassistant.cpp
    kgpgexternalactions.cpp
    selectexpirydate.cpp
    caff.cpp
    foldercompressjob.cpp
    misc.cpp
)

qt4_add_dbus_adaptor( kgpg_SRCS org.kde.kgpg.Key.xml keysmanager.h KeysManager)

QT4_ADD_DBUS_INTERFACE(kgpg_SRCS org.kde.kgpg.Key.xml kgpg_interface )

kde4_add_kcfg_files(kgpg_SRCS kgpgsettings.kcfgc )

add_executable(kgpg ${kgpg_SRCS})

if (KGPG_DEBUG_TRANSACTIONS)
    get_target_property(KGPG_FLAGS kgpg COMPILE_DEFINITIONS)
    if (NOT KGPG_FLAGS)
        # get rid of the NOTFOUND
        set(KGPG_FLAGS)
    endif (NOT KGPG_FLAGS)
    list(APPEND KGPG_FLAGS KGPG_DEBUG_TRANSACTIONS)
    set_target_properties(kgpg PROPERTIES COMPILE_DEFINITIONS "${KGPG_FLAGS}")
endif (KGPG_DEBUG_TRANSACTIONS)

target_link_libraries(kgpg
    ${KDE4_KIO_LIBS}
    ${KDE4_KABC_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${KDE4_KPIMUTILS_LIBRARY}
    ${KDE4_SOLID_LIBS}
)

install(TARGETS kgpg ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(
    FILES editor/kgpgeditor.rc keysmanager.rc tips
    DESTINATION ${KDE4_DATA_INSTALL_DIR}/kgpg
)
install(
    PROGRAMS kgpg.desktop
    DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR}
)
install(
    FILES kgpg.kcfg
    DESTINATION ${KDE4_KCFG_INSTALL_DIR}
)
install(
    FILES kgpg.desktop
    DESTINATION ${KDE4_AUTOSTART_INSTALL_DIR}
)
install(
    FILES encryptfile.desktop encryptfolder.desktop viewdecrypted.desktop
    DESTINATION ${KDE4_SERVICES_INSTALL_DIR}/ServiceMenus
)
install(
    FILES org.kde.kgpg.Key.xml
    DESTINATION ${KDE4_DBUS_INTERFACES_INSTALL_DIR}
)

kde4_install_icons(${KDE4_ICON_INSTALL_DIR})
