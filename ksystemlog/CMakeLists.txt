project(KSystemLog)

include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES})

########### Subfolders ##########

add_subdirectory(src)
add_subdirectory(src/lib)
add_subdirectory(src/config)
 
add_subdirectory(src/modes/base)
add_subdirectory(src/modes/open)
add_subdirectory(src/modes/system)
add_subdirectory(src/modes/kernel)
add_subdirectory(src/modes/xorg)
add_subdirectory(src/modes/cron)
add_subdirectory(src/modes/authentication)
add_subdirectory(src/modes/daemon)
add_subdirectory(src/modes/acpid)
add_subdirectory(src/modes/xsession)
add_subdirectory(src/modes/apache)
add_subdirectory(src/modes/postfix)
add_subdirectory(src/modes/samba)
add_subdirectory(src/modes/cups)

if(ENABLE_TESTING)
    add_subdirectory(tests)
endif()
