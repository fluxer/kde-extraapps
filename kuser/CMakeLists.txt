project(kuser)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    include(FeatureSummary)

    find_package(KDE4 4.23.0 REQUIRED)
    include(KDE4Defaults)
    include_directories(${KDE4_INCLUDES})
    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFiles)

add_subdirectory(pics)
add_subdirectory(toolbar)
add_subdirectory(icon)

# use the project-specific _SOURCE_DIR variable (set by the project() call above), Alex
include_directories(${kuser_SOURCE_DIR})

set(KU_HOMEDIR_PERM 0700 CACHE STRING "Default home directory permissions")
set(KU_MAILBOX_GID 0 CACHE STRING "Default GID of the created mbox file")
set(KU_MAILBOX_PERM 0660 CACHE STRING "Default mbox file permissions")
set(KU_HOMETEMPLATE "/home/%u" CACHE STRING "Default home directory template")
set(KU_FIRSTUID 500 CACHE STRING "First UID of normal users")
set(KU_FIRSTGID 500 CACHE STRING "First GID of normal users")
set(KU_USERPRIVATEGROUP true CACHE STRING "User private groups by default")

check_include_files(shadow.h HAVE_SHADOW_H)
check_include_files(crypt.h HAVE_CRYPT_H)
check_include_files(paths.h HAVE_PATHS_H)
check_library_exists(crypt crypt "" HAVE_CRYPT_LIBRARY)

set(CMAKE_REQUIRED_INCLUDES pwd.h)
check_function_exists(fgetpwent HAVE_FGETPWENT)
set(CMAKE_REQUIRED_INCLUDES grp.h)
check_function_exists(fgetgrent HAVE_FGETGRENT)
set(CMAKE_REQUIRED_INCLUDES shadow.h)
check_function_exists(fgetspent HAVE_FGETSPENT)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ku_config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/ku_config.h)


set(kuser_SRCS
    ku_mainwidget.cpp
    ku_mainview.cpp
    ku_edituser.cpp
    ku_editgroup.cpp
    ku_usermodel.cpp
    ku_groupmodel.cpp
    ku_deluser.cpp
    ku_user.cpp
    ku_group.cpp
    ku_main.cpp
    ku_misc.cpp
    ku_pwdlg.cpp
    ku_adduser.cpp
    ku_configdlg.cpp
    ku_global.cpp
    sid.cpp
    ku_userfiles.cpp
    ku_groupfiles.cpp
    ku_usersystem.cpp
    ku_groupsystem.cpp
    ku_selectconn.cpp
)

kde4_add_kcfg_files(kuser_SRCS ku_prefs.kcfgc)

add_executable(kuser ${kuser_SRCS})

target_link_libraries(kuser ${KDE4_KIO_LIBS})
if(HAVE_CRYPT_LIBRARY)
    target_link_libraries(kuser crypt)
endif(HAVE_CRYPT_LIBRARY)

install(TARGETS kuser ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(PROGRAMS kuser.desktop DESTINATION ${KDE4_XDG_APPS_INSTALL_DIR})
install(FILES kuserui.rc DESTINATION ${KDE4_DATA_INSTALL_DIR}/kuser)
install(FILES kuser.kcfg  DESTINATION ${KDE4_KCFG_INSTALL_DIR})

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
endif()
