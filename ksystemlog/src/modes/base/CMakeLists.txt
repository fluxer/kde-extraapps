
include_directories( 
	${CMAKE_CURRENT_SOURCE_DIR}/../..
	${CMAKE_CURRENT_SOURCE_DIR}/../../lib
	${CMAKE_CURRENT_BINARY_DIR}/../../config 
)

set(ksystemlog_base_mode_sources
	parsingHelper.cpp
	fileList.cpp
	logLevelFileList.cpp
	multipleFileList.cpp
	fileListHelper.cpp
	logLevelSelectionDialog.cpp
	genericConfiguration.cpp
	syslogAnalyzer.cpp
)

# avoid putting the generated files in target specific
# directory which is the case when automated
set(CMAKE_AUTOUIC OFF)

qt4_wrap_ui(ksystemlog_base_mode_sources
    fileListBase.ui
    multipleFileListBase.ui
    logLevelSelectionDialogBase.ui
)

add_library(ksystemlog_base_mode STATIC ${ksystemlog_base_mode_sources})

add_dependencies(
	ksystemlog_base_mode 
	ksystemlog_lib
)

target_link_libraries(
	ksystemlog_base_mode
	${KDE4_KDEUI_LIBS}
	ksystemlog_lib
	ksystemlog_config
)
