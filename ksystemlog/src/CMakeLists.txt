
########### Build ###############

include_directories( 
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_BINARY_DIR}/lib 
    ${CMAKE_CURRENT_SOURCE_DIR} 

    ${CMAKE_CURRENT_SOURCE_DIR}/modes/acpid
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/apache
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/authentication
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/base
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/cron
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/cups
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/daemon
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/open
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/postfix
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/samba
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/system
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/xorg
    ${CMAKE_CURRENT_SOURCE_DIR}/modes/xsession
)

set(ksystemlog_library_sources
    lib/analyzer.cpp
    lib/logModeFactory.cpp
    lib/logModeConfiguration.cpp
    lib/logModeConfigurationWidget.cpp
    lib/logModeItemBuilder.cpp
    lib/logManager.cpp
    lib/view.cpp
    lib/globals.cpp
    lib/logMode.cpp
    lib/logLevel.cpp
    lib/logFile.cpp
    lib/logFileReader.cpp
    lib/localLogFileReader.cpp
    lib/processOutputLogFileReader.cpp
    lib/kioLogFileReader.cpp
    lib/logLine.cpp
    lib/logViewModel.cpp
    lib/logViewColumn.cpp
    lib/logViewColumns.cpp
    lib/logViewWidgetItem.cpp
    lib/logViewWidget.cpp
    lib/logViewExport.cpp
    lib/logViewFilterWidget.cpp
    lib/logViewSearchWidget.cpp
    lib/loadingBar.cpp
    lib/logModeAction.cpp
    lib/simpleAction.cpp
    lib/multipleActions.cpp
    lib/levelPrintPage.cpp

    modes/base/parsingHelper.cpp
    modes/base/fileList.cpp
    modes/base/logLevelFileList.cpp
    modes/base/multipleFileList.cpp
    modes/base/fileListHelper.cpp
    modes/base/logLevelSelectionDialog.cpp
    modes/base/genericConfiguration.cpp
    modes/base/syslogAnalyzer.cpp
    modes/kernel/kernelFactory.cpp
    modes/kernel/kernelAnalyzer.cpp
    modes/kernel/kernelLogMode.cpp
    modes/kernel/kernelItemBuilder.cpp
    modes/system/systemFactory.cpp
    modes/system/systemConfigurationWidget.cpp
    modes/system/systemConfiguration.cpp
    modes/system/systemAnalyzer.cpp
    modes/system/systemLogMode.cpp
)

set(ksystemlog_sources
    modes/acpid/acpidConfigurationWidget.cpp
    modes/acpid/acpidConfiguration.cpp
    modes/acpid/acpidAnalyzer.cpp
    modes/acpid/acpidItemBuilder.cpp
    modes/acpid/acpidLogMode.cpp
    modes/acpid/acpidFactory.cpp
    modes/apache/apacheConfiguration.cpp
    modes/apache/apacheConfigurationWidget.cpp
    modes/apache/apacheAnalyzer.cpp
    modes/apache/apacheItemBuilder.cpp
    modes/apache/apacheAccessAnalyzer.cpp
    modes/apache/apacheAccessItemBuilder.cpp
    modes/apache/apacheFactory.cpp
    modes/apache/apacheLogMode.cpp
    modes/apache/apacheAccessLogMode.cpp
    modes/authentication/authenticationConfigurationWidget.cpp
    modes/authentication/authenticationConfiguration.cpp
    modes/authentication/authenticationAnalyzer.cpp
    modes/authentication/authenticationLogMode.cpp
    modes/authentication/authenticationFactory.cpp
    modes/cron/cronConfigurationWidget.cpp
    modes/cron/cronConfiguration.cpp
    modes/cron/cronAnalyzer.cpp
    modes/cron/cronItemBuilder.cpp
    modes/cron/cronLogMode.cpp
    modes/cron/cronFactory.cpp
    modes/cups/cupsConfiguration.cpp
    modes/cups/cupsConfigurationWidget.cpp
    modes/cups/cupsAnalyzer.cpp
    modes/cups/cupsItemBuilder.cpp
    modes/cups/cupsLogMode.cpp
    modes/cups/cupsAccessAnalyzer.cpp
    modes/cups/cupsAccessItemBuilder.cpp
    modes/cups/cupsAccessLogMode.cpp
    modes/cups/cupsPageAnalyzer.cpp
    modes/cups/cupsPageItemBuilder.cpp
    modes/cups/cupsPageLogMode.cpp
    modes/cups/cupsPdfAnalyzer.cpp
    modes/cups/cupsPdfItemBuilder.cpp
    modes/cups/cupsPdfLogMode.cpp
    modes/cups/cupsFactory.cpp
    modes/daemon/daemonConfigurationWidget.cpp
    modes/daemon/daemonConfiguration.cpp
    modes/daemon/daemonLogMode.cpp
    modes/daemon/daemonFactory.cpp
    modes/open/openAnalyzer.cpp
    modes/open/openFactory.cpp
    modes/open/openLogMode.cpp
    modes/postfix/postfixFactory.cpp
    modes/postfix/postfixConfigurationWidget.cpp
    modes/postfix/postfixConfiguration.cpp
    modes/postfix/postfixAnalyzer.cpp
    modes/postfix/postfixLogMode.cpp
    modes/samba/sambaConfiguration.cpp
    modes/samba/sambaConfigurationWidget.cpp
    modes/samba/sambaAnalyzer.cpp
    modes/samba/sambaLogMode.cpp
    modes/samba/sambaItemBuilder.cpp
    modes/samba/sambaAccessLogMode.cpp
    modes/samba/netbiosLogMode.cpp
    modes/samba/sambaFactory.cpp
    modes/xorg/xorgConfigurationWidget.cpp
    modes/xorg/xorgConfiguration.cpp
    modes/xorg/xorgAnalyzer.cpp
    modes/xorg/xorgItemBuilder.cpp
    modes/xorg/xorgLogMode.cpp
    modes/xorg/xorgFactory.cpp
    modes/xsession/xsessionConfigurationWidget.cpp
    modes/xsession/xsessionConfiguration.cpp
    modes/xsession/xsessionAnalyzer.cpp
    modes/xsession/xsessionItemBuilder.cpp
    modes/xsession/xsessionLogMode.cpp
    modes/xsession/xsessionFactory.cpp

    main.cpp
    mainWindow.cpp
    logModePluginsLoader.cpp
    loggerDialog.cpp
    detailDialog.cpp
    generalConfigurationWidget.cpp
    configurationDialog.cpp
    tabLogViewsWidget.cpp
    tabLogManager.cpp
    statusBar.cpp
)

kde4_add_kcfg_files(ksystemlog_library_sources ksystemlogConfig.kcfgc)

add_executable(ksystemlog ${ksystemlog_library_sources} ${ksystemlog_sources})

target_link_libraries(ksystemlog  
    ${KDE4_KIO_LIBS} 
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
)

if(ENABLE_TESTING)
    add_library(ksystemlog_library SHARED ${ksystemlog_library_sources})
endif()

########### Installation ###############

install(TARGETS ksystemlog ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS ksystemlog.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES ksystemlogui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/ksystemlog)
